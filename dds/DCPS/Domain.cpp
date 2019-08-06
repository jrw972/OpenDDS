/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "DCPS/DdsDcps_pch.h" //Only the _pch include should start with DCPS/
#include "Domain.h"

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

void LegacyDiscovery::association_complete(const DDS::DomainId_t domain_id,
                                           const RepoId& participant_id,
                                           const RepoId& local_id,
                                           const RepoId& remote_id) {
  discovery_->association_complete(domain_id, participant_id, local_id, remote_id);
}

bool LegacyDiscovery::update_publication_qos(const DDS::DomainId_t domain_id,
                                             const RepoId& participant_id,
                                             const RepoId& data_writer_id,
                                             const DDS::DataWriterQos& qos,
                                             const DDS::PublisherQos& publisher_qos) {
  return discovery_->update_publication_qos(domain_id, participant_id, data_writer_id, qos, publisher_qos);
}

void LegacyDiscovery::pre_writer(DataWriterImpl* data_writer) {
  discovery_->pre_writer(data_writer);
}

RepoId LegacyDiscovery::add_publication(const DDS::DomainId_t domain_id,
                                        const RepoId& participant_id,
                                        const RepoId& topic_id,
                                        DataWriterCallbacks* publication,
                                        const DDS::DataWriterQos& qos,
                                        const TransportLocatorSeq& trans_info,
                                        const DDS::PublisherQos& publisher_qos) {
  return discovery_->add_publication(domain_id, participant_id, topic_id, publication, qos, trans_info, publisher_qos);
}

bool LegacyDiscovery::supports_liveliness() const {
  return discovery_->supports_liveliness();
}

bool LegacyDiscovery::remove_publication(const DDS::DomainId_t domain_id,
                                         const RepoId& participant_id,
                                         const RepoId& publication_id) {
  return discovery_->remove_publication(domain_id, participant_id, publication_id);
}

bool LegacyDiscovery::update_subscription_qos(const DDS::DomainId_t domain_id,
                                              const RepoId& participant_id,
                                              const RepoId& data_reader_id,
                                              const DDS::DataReaderQos& qos,
                                              const DDS::SubscriberQos& subscriber_qos) {
  return discovery_->update_subscription_qos(domain_id, participant_id, data_reader_id, qos, subscriber_qos);
}

void LegacyDiscovery::pre_reader(DataReaderImpl* data_reader) {
  discovery_->pre_reader(data_reader);
}

RepoId LegacyDiscovery::add_subscription(const DDS::DomainId_t domain_id,
                                         const RepoId& participant_id,
                                         const RepoId& topic_id,
                                         DataReaderCallbacks* subscription,
                                         const DDS::DataReaderQos& qos,
                                         const TransportLocatorSeq& trans_info,
                                         const DDS::SubscriberQos& subscriber_qos,
                                         const char* filter_class_name,
                                         const char* filter_expression,
                                         const DDS::StringSeq& expr_params) {
  return discovery_->add_subscription(domain_id, participant_id, topic_id, subscription, qos, trans_info, subscriber_qos, filter_class_name, filter_expression, expr_params);
}

RepoId LegacyDiscovery::bit_key_to_repo_id(DomainParticipantImpl* participant,
                                           const char* bit_topic_name,
                                           const DDS::BuiltinTopicKey_t& key) const {
  return discovery_->bit_key_to_repo_id(participant, bit_topic_name, key);
}

bool LegacyDiscovery::update_subscription_params(const DDS::DomainId_t domain_id,
                                                 const RepoId& participant_id,
                                                 const RepoId& subscription_id,
                                                 const DDS::StringSeq& params) {
  return discovery_->update_subscription_params(domain_id, participant_id, subscription_id, params);
}

bool LegacyDiscovery::remove_subscription(const DDS::DomainId_t domain_id,
                                          const RepoId& participant_id,
                                          const RepoId& subscription_id) {
  return discovery_->remove_subscription(domain_id, participant_id, subscription_id);
}

bool LegacyDiscovery::remove_domain_participant(const DDS::DomainId_t domain_id,
                                                const RepoId& participant_id) {
  return discovery_->remove_domain_participant(domain_id, participant_id);
}

bool LegacyDiscovery::update_topic_qos(const DDS::DomainId_t domain_id,
                                       const RepoId& topic_id,
                                       const RepoId& participant_id,
                                       const DDS::TopicQos& qos) {
  return discovery_->update_topic_qos(topic_id, domain_id, participant_id, qos);
}

TopicStatus LegacyDiscovery::assert_topic(const DDS::DomainId_t domain_id,
                                          RepoId_out topic_id,
                                          const RepoId& participant_id,
                                          const char* topic_name,
                                          const char* data_type_name,
                                          const DDS::TopicQos& qos,
                                          bool has_dcps_key,
                                          TopicCallbacks* topic_callbacks) {
  return discovery_->assert_topic(topic_id, domain_id, participant_id, topic_name, data_type_name, qos, has_dcps_key, topic_callbacks);
}

TopicStatus LegacyDiscovery::remove_topic(const DDS::DomainId_t domain_id,
                                          const RepoId& participant_id,
                                          const RepoId& topic_id) {
  return discovery_->remove_topic(domain_id, participant_id, topic_id);
}

TopicStatus LegacyDiscovery::find_topic(const DDS::DomainId_t domain_id,
                                        const RepoId& participant_id,
                                        const char* topic_name,
                                        CORBA::String_out data_type_name,
                                        DDS::TopicQos_out qos,
                                        RepoId_out topic_id) {
  return discovery_->find_topic(domain_id, participant_id, topic_name, data_type_name, qos, topic_id);
}

void LegacyDiscovery::fini_bit(DCPS::DomainParticipantImpl* participant) {
  discovery_->fini_bit(participant);
}

bool LegacyDiscovery::update_domain_participant_qos(const DDS::DomainId_t domain_id,
                                                    const RepoId& participant_id,
                                                    const DDS::DomainParticipantQos& qos) {
  return discovery_->update_domain_participant_qos(domain_id, participant_id, qos);
}

bool LegacyDiscovery::ignore_domain_participant(const DDS::DomainId_t domain_id,
                                                const RepoId& participant_id,
                                                const RepoId& ignore_id) {
  return discovery_->ignore_domain_participant(domain_id, participant_id, ignore_id);
}

bool LegacyDiscovery::ignore_topic(const DDS::DomainId_t domain_id,
                                   const RepoId& participant_id,
                                   const RepoId& ignore_id) {
  return discovery_->ignore_topic(domain_id, participant_id, ignore_id);
}

bool LegacyDiscovery::ignore_publication(const DDS::DomainId_t domain_id,
                                         const RepoId& participant_id,
                                         const RepoId& ignore_id) {
  return discovery_->ignore_publication(domain_id, participant_id, ignore_id);
}

bool LegacyDiscovery::ignore_subscription(const DDS::DomainId_t domain_id,
                                          const RepoId& participant_id,
                                          const RepoId& ignore_id) {
  return discovery_->ignore_subscription(domain_id, participant_id, ignore_id);
}

OpenDDS::DCPS::RepoId LegacyDiscovery::generate_participant_guid() {
  return discovery_->generate_participant_guid();
}

AddDomainStatus LegacyDiscovery::add_domain_participant(const DDS::DomainId_t domain_id,
                                                        const DDS::DomainParticipantQos& qos) {
  return discovery_->add_domain_participant(domain_id, qos);
}

#if defined(OPENDDS_SECURITY)
AddDomainStatus LegacyDiscovery::add_domain_participant_secure(const DDS::DomainId_t domain_id,
                                                               const DDS::DomainParticipantQos& qos,
                                                               const OpenDDS::DCPS::RepoId& guid,
                                                               DDS::Security::IdentityHandle id,
                                                               DDS::Security::PermissionsHandle perm,
                                                               DDS::Security::ParticipantCryptoHandle part_crypto) {
  return discovery_->add_domain_participant_secure(domain_id, qos, guid, id, perm, part_crypto);
}
#endif

DDS::Subscriber_ptr LegacyDiscovery::init_bit(DomainParticipantImpl* participant,
                                              Domain* domain) {
  return discovery_->init_bit(participant, domain);
}

void LegacyDiscovery::signal_liveliness(const DDS::DomainId_t domain_id,
                                        const RepoId& participant_id,
                                        DDS::LivelinessQosPolicyKind kind) {
  discovery_->signal_liveliness(domain_id, participant_id, kind);
}


void Domain::association_complete(const RepoId& participant_id,
				  const RepoId& local_id,
				  const RepoId& remote_id) {
  discovery_->association_complete(domain_id_, participant_id, local_id, remote_id);
}

bool Domain::update_publication_qos(const RepoId& participant_id,
				    const RepoId& data_writer_id,
				    const DDS::DataWriterQos& qos,
				    const DDS::PublisherQos& publisher_qos) {
  return discovery_->update_publication_qos(domain_id_, participant_id, data_writer_id, qos, publisher_qos);
}

void Domain::pre_writer(DataWriterImpl* data_writer) {
  discovery_->pre_writer(data_writer);
}

RepoId Domain::add_publication(const RepoId& participant_id,
			       const RepoId& topic_id,
			       DataWriterCallbacks* publication,
			       const DDS::DataWriterQos& qos,
			       const TransportLocatorSeq& trans_info,
			       const DDS::PublisherQos& publisher_qos) {
  return discovery_->add_publication(domain_id_, participant_id, topic_id, publication, qos, trans_info, publisher_qos);
}

bool Domain::supports_liveliness() const {
  return discovery_->supports_liveliness();
}

bool Domain::remove_publication(const RepoId& participant_id,
				const RepoId& publication_id) {
  return discovery_->remove_publication(domain_id_, participant_id, publication_id);
}

bool Domain::update_subscription_qos(const RepoId& participant_id,
				     const RepoId& data_reader_id,
				     const DDS::DataReaderQos& qos,
				     const DDS::SubscriberQos& subscriber_qos) {
  return discovery_->update_subscription_qos(domain_id_, participant_id, data_reader_id, qos, subscriber_qos);
}

void Domain::pre_reader(DataReaderImpl* data_reader) {
  discovery_->pre_reader(data_reader);
}

RepoId Domain::add_subscription(const RepoId& participant_id,
				const RepoId& topic_id,
				DataReaderCallbacks* subscription,
				const DDS::DataReaderQos& qos,
				const TransportLocatorSeq& trans_info,
				const DDS::SubscriberQos& subscriber_qos,
				const char* filter_class_name,
				const char* filter_expression,
				const DDS::StringSeq& expr_params) {
  return discovery_->add_subscription(domain_id_, participant_id, topic_id, subscription, qos, trans_info, subscriber_qos, filter_class_name, filter_expression, expr_params);
}

RepoId Domain::bit_key_to_repo_id(DomainParticipantImpl* participant,
				  const char* bit_topic_name,
				  const DDS::BuiltinTopicKey_t& key) const {
  return discovery_->bit_key_to_repo_id(participant, bit_topic_name, key);
}

bool Domain::update_subscription_params(const RepoId& participant_id,
					const RepoId& subscription_id,
					const DDS::StringSeq& params) {
  return discovery_->update_subscription_params(domain_id_, participant_id, subscription_id, params);
}

bool Domain::remove_subscription(const RepoId& participant_id,
				 const RepoId& subscription_id) {
  return discovery_->remove_subscription(domain_id_, participant_id, subscription_id);
}

bool Domain::remove_domain_participant(const RepoId& participant_id) {
  return discovery_->remove_domain_participant(domain_id_, participant_id);
}

bool Domain::update_topic_qos(const RepoId& topic_id,
			      const RepoId& participant_id,
			      const DDS::TopicQos& qos) {
  return discovery_->update_topic_qos(domain_id_, topic_id, participant_id, qos);
}

TopicStatus Domain::assert_topic(RepoId_out topic_id,
				 const RepoId& participant_id,
				 const char* topic_name,
				 const char* data_type_name,
				 const DDS::TopicQos& qos,
				 bool has_dcps_key,
				 TopicCallbacks* topic_callbacks) {
  return discovery_->assert_topic(domain_id_, topic_id, participant_id, topic_name, data_type_name, qos, has_dcps_key, topic_callbacks);
}

TopicStatus Domain::remove_topic(const RepoId& participant_id,
				 const RepoId& topic_id) {
  return discovery_->remove_topic(domain_id_, participant_id, topic_id);
}

TopicStatus Domain::find_topic(const RepoId& participant_id,
			       const char* topic_name,
			       CORBA::String_out data_type_name,
			       DDS::TopicQos_out qos,
			       RepoId_out topic_id) {
  return discovery_->find_topic(domain_id_, participant_id, topic_name, data_type_name, qos, topic_id);
}

void Domain::fini_bit(DCPS::DomainParticipantImpl* participant) {
  discovery_->fini_bit(participant);
}

bool Domain::update_domain_participant_qos(const RepoId& participant_id,
					   const DDS::DomainParticipantQos& qos) {
  return discovery_->update_domain_participant_qos(domain_id_, participant_id, qos);
}

bool Domain::ignore_domain_participant(const RepoId& participant_id,
				       const RepoId& ignore_id) {
  return discovery_->ignore_domain_participant(domain_id_, participant_id, ignore_id);
}

bool Domain::ignore_topic(const RepoId& participant_id,
			  const RepoId& ignore_id) {
  return discovery_->ignore_topic(domain_id_, participant_id, ignore_id);
}

bool Domain::ignore_publication(const RepoId& participant_id,
				const RepoId& ignore_id) {
  return discovery_->ignore_publication(domain_id_, participant_id, ignore_id);
}

bool Domain::ignore_subscription(const RepoId& participant_id,
				 const RepoId& ignore_id) {
  return discovery_->ignore_subscription(domain_id_, participant_id, ignore_id);
}

OpenDDS::DCPS::RepoId Domain::generate_participant_guid() {
  return discovery_->generate_participant_guid();
}

AddDomainStatus Domain::add_domain_participant(const DDS::DomainParticipantQos& qos) {
  return discovery_->add_domain_participant(domain_id_, qos);
}

#if defined(OPENDDS_SECURITY)
AddDomainStatus Domain::add_domain_participant_secure(
						      const DDS::DomainParticipantQos& qos,
						      const OpenDDS::DCPS::RepoId& guid,
						      DDS::Security::IdentityHandle id,
						      DDS::Security::PermissionsHandle perm,
						      DDS::Security::ParticipantCryptoHandle part_crypto) {
  return discovery_->add_domain_participant_secure(domain_id_, qos, guid, id, perm, part_crypto);
}
#endif

DDS::Subscriber_ptr Domain::init_bit(DomainParticipantImpl* participant) {
  return discovery_->init_bit(participant, this);
}

void Domain::signal_liveliness(const RepoId& participant_id,
			       DDS::LivelinessQosPolicyKind kind) {
  discovery_->signal_liveliness(domain_id_, participant_id, kind);
}

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
