/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "RtpsDiscovery.h"

#include "dds/DCPS/Service_Participant.h"
#include "dds/DCPS/ConfigUtils.h"
#include "dds/DCPS/DomainParticipantImpl.h"
#include "dds/DCPS/SubscriberImpl.h"
#include "dds/DCPS/Marked_Default_Qos.h"
#include "dds/DCPS/BuiltInTopicUtils.h"
#include "dds/DCPS/Registered_Data_Types.h"
#include "dds/DdsDcpsInfoUtilsC.h"

#include "ace/Reactor.h"
#include "ace/Select_Reactor.h"

#include <cstdlib>

namespace {
  u_short get_default_d0(u_short fallback)
  {
#if !defined ACE_LACKS_GETENV && !defined ACE_LACKS_ENV
    const char* from_env = std::getenv("OPENDDS_RTPS_DEFAULT_D0");
    if (from_env) {
      return static_cast<u_short>(std::atoi(from_env));
    }
#endif
    return fallback;
  }
}

namespace Util {

  template <typename Key>
  int find(
    OpenDDS::DCPS::DomainParticipantImpl::TopicMap& c,
    const Key& key,
    OpenDDS::DCPS::DomainParticipantImpl::TopicMap::mapped_type*& value)
  {
    OpenDDS::DCPS::DomainParticipantImpl::TopicMap::iterator iter =
      c.find(key);

    if (iter == c.end()) {
      return -1;
    }

    value = &iter->second;
    return 0;
  }

  DDS::PropertySeq filter_properties(const DDS::PropertySeq& properties, const std::string& prefix)
  {
    DDS::PropertySeq result(properties.length());
    result.length(properties.length());
    unsigned int count = 0;
    for (unsigned int i = 0; i < properties.length(); ++i) {
      if (std::string(properties[i].name.in()).find(prefix) == 0) {
        result[count++] = properties[i];
      }
    }
    result.length(count);
    return result;
  }

} // namespace Util

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace RTPS {

RtpsDiscovery::RtpsDiscovery(const RepoKey& key)
  : DCPS::PeerDiscovery<Spdp>(key)
  , resend_period_(30 /*seconds*/) // see RTPS v2.1 9.6.1.4.2
  , pb_(7400) // see RTPS v2.1 9.6.1.3 for PB, DG, PG, D0, D1 defaults
  , dg_(250)
  , pg_(2)
  , d0_(get_default_d0(0))
  , d1_(10)
  , dx_(2)
  , ttl_(1)
  , sedp_multicast_(true)
  , default_multicast_group_("239.255.0.1") /*RTPS v2.1 9.6.1.4.1*/
  , use_ice_(false)
  , max_spdp_timer_period_(0, 10000)
  , max_auth_time_(300, 0)
  , auth_resend_period_(1, 0)
{
}

RtpsDiscovery::~RtpsDiscovery()
{
}

namespace {
  const ACE_TCHAR RTPS_SECTION_NAME[] = ACE_TEXT("rtps_discovery");
}

int
RtpsDiscovery::Config::discovery_config(ACE_Configuration_Heap& cf)
{
  const ACE_Configuration_Section_Key &root = cf.root_section();
  ACE_Configuration_Section_Key rtps_sect;

  if (cf.open_section(root, RTPS_SECTION_NAME, 0, rtps_sect) == 0) {

    // Ensure there are no properties in this section
    DCPS::ValueMap vm;
    if (DCPS::pullValues(cf, rtps_sect, vm) > 0) {
      // There are values inside [rtps_discovery]
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
                        ACE_TEXT("rtps_discovery sections must have a subsection name\n")),
                       -1);
    }
    // Process the subsections of this section (the individual rtps_discovery/*)
    DCPS::KeyList keys;
    if (DCPS::processSections(cf, rtps_sect, keys) != 0) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
                        ACE_TEXT("too many nesting layers in the [rtps] section.\n")),
                       -1);
    }

    // Loop through the [rtps_discovery/*] sections
    for (DCPS::KeyList::const_iterator it = keys.begin();
         it != keys.end(); ++it) {
      const OPENDDS_STRING& rtps_name = it->first;

      RtpsDiscovery_rch discovery = OpenDDS::DCPS::make_rch<RtpsDiscovery>(rtps_name);

      // spdpaddr defaults to DCPSDefaultAddress if set
      if (!TheServiceParticipant->default_address().empty()) {
        discovery->spdp_local_address(TheServiceParticipant->default_address().c_str());
      }

      DCPS::ValueMap values;
      DCPS::pullValues(cf, it->second, values);
      for (DCPS::ValueMap::const_iterator it = values.begin();
           it != values.end(); ++it) {
        const OPENDDS_STRING& name = it->first;
        if (name == "ResendPeriod") {
          const OPENDDS_STRING& value = it->second;
          int resend;
          if (!DCPS::convertToInteger(value, resend)) {
            ACE_ERROR_RETURN((LM_ERROR,
              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
              ACE_TEXT("Invalid entry (%C) for ResendPeriod in ")
              ACE_TEXT("[rtps_discovery/%C] section.\n"),
              value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->resend_period(ACE_Time_Value(resend));
        } else if (name == "PB") {
          const OPENDDS_STRING& value = it->second;
          u_short pb;
          if (!DCPS::convertToInteger(value, pb)) {
            ACE_ERROR_RETURN((LM_ERROR,
              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
              ACE_TEXT("Invalid entry (%C) for PB in ")
              ACE_TEXT("[rtps_discovery/%C] section.\n"),
              value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->pb(pb);
        } else if (name == "DG") {
          const OPENDDS_STRING& value = it->second;
          u_short dg;
          if (!DCPS::convertToInteger(value, dg)) {
            ACE_ERROR_RETURN((LM_ERROR,
              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
              ACE_TEXT("Invalid entry (%C) for DG in ")
              ACE_TEXT("[rtps_discovery/%C] section.\n"),
              value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->dg(dg);
        } else if (name == "PG") {
          const OPENDDS_STRING& value = it->second;
          u_short pg;
          if (!DCPS::convertToInteger(value, pg)) {
            ACE_ERROR_RETURN((LM_ERROR,
              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
              ACE_TEXT("Invalid entry (%C) for PG in ")
              ACE_TEXT("[rtps_discovery/%C] section.\n"),
              value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->pg(pg);
        } else if (name == "D0") {
          const OPENDDS_STRING& value = it->second;
          u_short d0;
          if (!DCPS::convertToInteger(value, d0)) {
            ACE_ERROR_RETURN((LM_ERROR,
              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
              ACE_TEXT("Invalid entry (%C) for D0 in ")
              ACE_TEXT("[rtps_discovery/%C] section.\n"),
              value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->d0(d0);
        } else if (name == "D1") {
          const OPENDDS_STRING& value = it->second;
          u_short d1;
          if (!DCPS::convertToInteger(value, d1)) {
            ACE_ERROR_RETURN((LM_ERROR,
              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
              ACE_TEXT("Invalid entry (%C) for D1 in ")
              ACE_TEXT("[rtps_discovery/%C] section.\n"),
              value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->d1(d1);
        } else if (name == "DX") {
          const OPENDDS_STRING& value = it->second;
          u_short dx;
          if (!DCPS::convertToInteger(value, dx)) {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for DX in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->dx(dx);
        } else if (name == "TTL") {
          const OPENDDS_STRING& value = it->second;
          unsigned short ttl_us;
          if (!DCPS::convertToInteger(value, ttl_us) || ttl_us > UCHAR_MAX) {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for TTL in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->ttl(static_cast<unsigned char>(ttl_us));
        } else if (name == "SedpMulticast") {
          const OPENDDS_STRING& value = it->second;
          int smInt;
          if (!DCPS::convertToInteger(value, smInt)) {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config ")
               ACE_TEXT("Invalid entry (%C) for SedpMulticast in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->sedp_multicast(bool(smInt));
        } else if (name == "MulticastInterface") {
          discovery->multicast_interface(it->second);
        } else if (name == "SedpLocalAddress") {
          discovery->sedp_local_address(it->second);
        } else if (name == "SpdpLocalAddress") {
          discovery->spdp_local_address(it->second);
        } else if (name == "GuidInterface") {
          discovery->guid_interface(it->second);
        } else if (name == "InteropMulticastOverride") {
          /// FUTURE: handle > 1 group.
          discovery->default_multicast_group(it->second);
        } else if (name == "SpdpSendAddrs") {
          AddrVec spdp_send_addrs;
          const OPENDDS_STRING& value = it->second;
          size_t i = 0;
          do {
            i = value.find_first_not_of(' ', i); // skip spaces
            const size_t n = value.find_first_of(", ", i);
            spdp_send_addrs.push_back(value.substr(i, (n == OPENDDS_STRING::npos) ? n : n - i));
            i = value.find(',', i);
          } while (i++ != OPENDDS_STRING::npos); // skip past comma if there is one
          discovery->spdp_send_addrs().swap(spdp_send_addrs);
        } else if (name == "SpdpRtpsRelayAddress") {
          discovery->spdp_rtps_relay_address(ACE_INET_Addr(it->second.c_str()));
        } else if (name == "SedpRtpsRelayAddress") {
          discovery->sedp_rtps_relay_address(ACE_INET_Addr(it->second.c_str()));
#ifdef OPENDDS_SECURITY
        } else if (name == "SedpStunServerAddress") {
          discovery->sedp_stun_server_address(ACE_INET_Addr(it->second.c_str()));
        } else if (name == "UseIce") {
          const OPENDDS_STRING& value = it->second;
          int smInt;
          if (!DCPS::convertToInteger(value, smInt)) {
            ACE_ERROR_RETURN((LM_ERROR,
                              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config ")
                              ACE_TEXT("Invalid entry (%C) for UseIce in ")
                              ACE_TEXT("[rtps_discovery/%C] section.\n"),
                              value.c_str(), rtps_name.c_str()), -1);
          }
          discovery->use_ice(bool(smInt));
        } else if (name == "IceTa") {
          // In milliseconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().T_a(ACE_Time_Value(0, int_value * 1000));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceTa in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceConnectivityCheckTTL") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().connectivity_check_ttl(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceConnectivityCheckTTL in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceChecklistPeriod") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().checklist_period(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceChecklistPeriod in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceIndicationPeriod") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().indication_period(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceIndicationPeriod in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceNominatedTTL") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().nominated_ttl(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceNominatedTTL in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceServerReflexiveAddressPeriod") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().server_reflexive_address_period(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceServerReflexiveAddressPeriod in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceServerReflexiveIndicationCount") {
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().server_reflexive_indication_count(int_value);
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceServerReflexiveIndicationCount in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceDeferredTriggeredCheckTTL") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().deferred_triggered_check_ttl(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceDeferredTriggeredCheckTTL in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "IceChangePasswordPeriod") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            ICE::Agent::instance()->get_configuration().change_password_period(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
               ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
               ACE_TEXT("Invalid entry (%C) for IceChangePasswordPeriod in ")
               ACE_TEXT("[rtps_discovery/%C] section.\n"),
               string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "MaxAuthTime") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            discovery->max_auth_time(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
                              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
                              ACE_TEXT("Invalid entry (%C) for MaxAuthTime in ")
                              ACE_TEXT("[rtps_discovery/%C] section.\n"),
                              string_value.c_str(), rtps_name.c_str()), -1);
          }
        } else if (name == "AuthResendPeriod") {
          // In seconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            discovery->auth_resend_period(ACE_Time_Value(int_value));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
                              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
                              ACE_TEXT("Invalid entry (%C) for AuthResendPeriod in ")
                              ACE_TEXT("[rtps_discovery/%C] section.\n"),
                              string_value.c_str(), rtps_name.c_str()), -1);
          }
#endif /* OPENDDS_SECURITY */
        } else if (name == "MaxSpdpTimerPeriod") {
          // In milliseconds.
          const OPENDDS_STRING& string_value = it->second;
          int int_value;
          if (DCPS::convertToInteger(string_value, int_value)) {
            discovery->max_spdp_timer_period(ACE_Time_Value(0, int_value * 1000));
          } else {
            ACE_ERROR_RETURN((LM_ERROR,
                              ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
                              ACE_TEXT("Invalid entry (%C) for MaxSpdpTimerPeriod in ")
                              ACE_TEXT("[rtps_discovery/%C] section.\n"),
                              string_value.c_str(), rtps_name.c_str()), -1);
          }
        }  else {
          ACE_ERROR_RETURN((LM_ERROR,
                            ACE_TEXT("(%P|%t) RtpsDiscovery::Config::discovery_config(): ")
                            ACE_TEXT("Unexpected entry (%C) in [rtps_discovery/%C] section.\n"),
                            name.c_str(), rtps_name.c_str()),
                           -1);
        }
      }

      TheServiceParticipant->add_discovery(discovery);
    }
  }

  // If the default RTPS discovery object has not been configured,
  // instantiate it now.
  const DCPS::Service_Participant::RepoKeyDiscoveryMap& discoveryMap = TheServiceParticipant->discoveryMap();
  if (discoveryMap.find(Discovery::DEFAULT_RTPS) == discoveryMap.end()) {
    TheServiceParticipant->add_discovery(OpenDDS::DCPS::make_rch<RtpsDiscovery>(Discovery::DEFAULT_RTPS));
  }

  return 0;
}

// Participant operations:

DDS::ReturnCode_t
RtpsDiscovery::add_domain_participant(DDS::DomainId_t domain_id,
                                      DCPS::DomainParticipantImpl* dp)
{
  OpenDDS::DCPS::RepoId id = GUID_UNKNOWN;
  ACE_GUARD_RETURN(ACE_Thread_Mutex, g, lock_, DDS::RETCODE_ERROR);
  if (!guid_interface_.empty()) {
    if (guid_gen_.interfaceName(guid_interface_.c_str()) != 0) {
      if (DCPS::DCPS_debug_level) {
        ACE_DEBUG((LM_WARNING, "(%P|%t) RtpsDiscovery::add_domain_participant()"
                   " - attempt to use specific network interface's MAC addr for"
                   " GUID generation failed.\n"));
      }
    }
  }
  guid_gen_.populate(id);
  id.entityId = ENTITYID_PARTICIPANT;

#ifdef OPENDDS_SECURITY
  if (TheServiceParticipant->get_security()) {
    Security::Authentication_var auth = dp->get_security_config()->get_authentication();

    DDS::Security::SecurityException se;
    DDS::Security::ValidationResult_t val_res =
      auth->validate_local_identity(dp->id_handle(), id, domain_id, dp->qos(), id, se);

    /* TODO - Handle VALIDATION_PENDING_RETRY */
    if (val_res != DDS::Security::VALIDATION_OK) {
      ACE_ERROR((LM_ERROR,
        ACE_TEXT("(%P|%t) ERROR: ")
        ACE_TEXT("DomainParticipantImpl::enable, ")
        ACE_TEXT("Unable to validate local identity. SecurityException[%d.%d]: %C\n"),
          se.code, se.minor_code, se.message.in()));
      return DDS::Security::RETCODE_NOT_ALLOWED_BY_SECURITY;
    }

    Security::AccessControl_var access = dp->get_security_config()->get_access_control();

    dp->perm_handle() = access->validate_local_permissions(auth, dp->id_handle(), domain_id, dp->qos(), se);

    if (dp->perm_handle() == DDS::HANDLE_NIL) {
      ACE_ERROR((LM_ERROR,
        ACE_TEXT("(%P|%t) ERROR: ")
        ACE_TEXT("DomainParticipantImpl::enable, ")
        ACE_TEXT("Unable to validate local permissions. SecurityException[%d.%d]: %C\n"),
          se.code, se.minor_code, se.message.in()));
      return DDS::Security::RETCODE_NOT_ALLOWED_BY_SECURITY;
    }

    bool check_create = access->check_create_participant(dp->perm_handle(), domain_id, dp->qos(), se);
    if (!check_create) {
      ACE_ERROR((LM_ERROR,
        ACE_TEXT("(%P|%t) ERROR: ")
        ACE_TEXT("DomainParticipantImpl::enable, ")
        ACE_TEXT("Unable to create participant. SecurityException[%d.%d]: %C\n"),
          se.code, se.minor_code, se.message.in()));
      return DDS::Security::RETCODE_NOT_ALLOWED_BY_SECURITY;
    }

    DDS::Security::ParticipantSecurityAttributes part_sec_attr;
    bool check_part_sec_attr = access->get_participant_sec_attributes(dp->perm_handle(), part_sec_attr, se);

    if (!check_part_sec_attr) {
      ACE_ERROR((LM_ERROR,
        ACE_TEXT("(%P|%t) ERROR: ")
        ACE_TEXT("DomainParticipantImpl::enable, ")
        ACE_TEXT("Unable to get participant security attributes. SecurityException[%d.%d]: %C\n"),
          se.code, se.minor_code, se.message.in()));
      return DDS::RETCODE_ERROR;
    }

    Security::CryptoKeyFactory_var crypto = dp->get_security_config()->get_crypto_key_factory();

    dp->crypto_handle() = crypto->register_local_participant(dp->id_handle(), dp->perm_handle(),
      Util::filter_properties(dp->qos().property.value, "dds.sec.crypto."), part_sec_attr, se);
    if (dp->crypto_handle() == DDS::HANDLE_NIL) {
      ACE_ERROR((LM_ERROR,
        ACE_TEXT("(%P|%t) ERROR: ")
        ACE_TEXT("DomainParticipantImpl::enable, ")
        ACE_TEXT("Unable to register local participant. SecurityException[%d.%d]: %C\n"),
          se.code, se.minor_code, se.message.in()));
      return DDS::RETCODE_ERROR;
    }

    id.entityId = ENTITYID_PARTICIPANT;
    try {
      const DCPS::RcHandle<Spdp> spdp (DCPS::make_rch<Spdp>(domain_id, id, dp->qos(), this, dp->id_handle(), dp->perm_handle(), dp->crypto_handle()));
      participants_[domain_id][id] = spdp;
    } catch (const std::exception& e) {
      ACE_ERROR((LM_WARNING, "(%P|%t) RtpsDiscovery::add_domain_participant_secure() - "
                 "failed to initialize RTPS Simple Participant Discovery Protocol: %C\n",
                 e.what()));
      return DDS::RETCODE_ERROR;
    }
  } else {
#endif

    try {
      const DCPS::RcHandle<Spdp> spdp (DCPS::make_rch<Spdp>(domain_id, ref(id), dp->qos(), this));
      // ads.id may change during Spdp constructor
      participants_[domain_id][id] = spdp;
    } catch (const std::exception& e) {
      ACE_ERROR((LM_ERROR, "(%P|%t) RtpsDiscovery::add_domain_participant() - "
                 "failed to initialize RTPS Simple Participant Discovery Protocol: %C\n",
                 e.what()));
      return DDS::RETCODE_ERROR;
    }
#ifdef OPENDDS_SECURITY
  }
#endif

  dp->set_id(id);
  return DDS::RETCODE_OK;
}

void
RtpsDiscovery::signal_liveliness(const DDS::DomainId_t domain_id,
                                 const OpenDDS::DCPS::RepoId& part_id,
                                 DDS::LivelinessQosPolicyKind kind)
{
  get_part(domain_id, part_id)->signal_liveliness(kind);
}

RtpsDiscovery::StaticInitializer::StaticInitializer()
{
  TheServiceParticipant->register_discovery_type("rtps_discovery", new Config);
}

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
