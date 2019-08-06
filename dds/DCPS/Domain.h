/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef OPENDDS_DCPS_DOMAIN_H
#define OPENDDS_DCPS_DOMAIN_H

#include "Definitions.h"
#include "dds/DCPS/Discovery.h"
#include "dds/DdsDcpsDomainC.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

class DiscoveryI {
 public:
  virtual ~DiscoveryI() {}

  // Participant
  virtual RepoId generate_participant_guid() = 0;

  // TODO(jrw972): Remove AddDomainStatus.
  virtual AddDomainStatus add_domain_participant(const DDS::DomainId_t domain_id,
                                                 const DDS::DomainParticipantQos& qos) = 0;

#if defined(OPENDDS_SECURITY)
  // TODO(jrw972): Remove conditional declaration.
  virtual AddDomainStatus add_domain_participant_secure(const DDS::DomainId_t domain_id,
                                                        const DDS::DomainParticipantQos& qos,
                                                        const OpenDDS::DCPS::RepoId& guid,
                                                        DDS::Security::IdentityHandle id,
                                                        DDS::Security::PermissionsHandle perm,
                                                        DDS::Security::ParticipantCryptoHandle part_crypto) = 0;
#endif

  virtual bool update_domain_participant_qos(const DDS::DomainId_t domain_id,
                                             const RepoId& participant_id,
                                             const DDS::DomainParticipantQos& qos) = 0;

  virtual bool remove_domain_participant(const DDS::DomainId_t domain_id,
                                         const RepoId& participant_id) = 0;

  virtual bool ignore_domain_participant(const DDS::DomainId_t domain_id,
                                         const RepoId& participant_id,
                                         const RepoId& ignore_id) = 0;

  virtual bool ignore_topic(const DDS::DomainId_t domain_id,
                            const RepoId& participant_id,
                            const RepoId& ignore_id) = 0;

  virtual bool ignore_publication(const DDS::DomainId_t domain_id,
                                  const RepoId& participant_id,
                                  const RepoId& ignore_id) = 0;

  virtual bool ignore_subscription(const DDS::DomainId_t domain_id,
                                   const RepoId& participant_id,
                                   const RepoId& ignore_id) = 0;

  // TODO(jrw972): Decide who has the responsibility of supporting liveliness.
  virtual bool supports_liveliness() const = 0;

  virtual void signal_liveliness(const DDS::DomainId_t domain_id,
                                 const RepoId& participant_id,
                                 DDS::LivelinessQosPolicyKind kind) = 0;

  virtual RepoId bit_key_to_repo_id(DomainParticipantImpl* participant,
                                    const char* bit_topic_name,
                                    const DDS::BuiltinTopicKey_t& key) const = 0;

  // TODO(jrs972): Passing the domain is a hack.  Once the datareaders
  // are decoupled from the transport, the participants themselves can
  // create the builtin topics with its implicit domain.
  virtual DDS::Subscriber_ptr init_bit(DomainParticipantImpl* participant,
                                       Domain* domain) = 0;

  virtual void fini_bit(DCPS::DomainParticipantImpl* participant) = 0;

  // Topics
  // TODO(jrw972): Check what TopicStatus is providing.  See if these two functions can look more orthodox.
  virtual TopicStatus assert_topic(const DDS::DomainId_t domain_id,
                                   RepoId_out topic_id,
                                   const RepoId& participant_id,
                                   const char* topic_name,
                                   const char* data_type_name,
                                   const DDS::TopicQos& qos,
                                   bool has_dcps_key,
                                   TopicCallbacks* topic_callbacks) = 0;

  virtual TopicStatus find_topic(const DDS::DomainId_t domain_id,
                                 const RepoId& participant_id,
                                 const char* topic_name,
                                 CORBA::String_out data_type_name,
                                 DDS::TopicQos_out qos,
                                 RepoId_out topic_id) = 0;

  virtual bool update_topic_qos(const DDS::DomainId_t domain_id,
                                const RepoId& topic_id,
                                const RepoId& participant_id,
                                const DDS::TopicQos& qos) = 0;

  virtual TopicStatus remove_topic(const DDS::DomainId_t domain_id,
                                   const RepoId& participant_id,
                                   const RepoId& topic_id) = 0;

  // Publications
  // TODO(jrw972): Why?
  virtual void pre_writer(DataWriterImpl* data_writer) = 0;

  virtual RepoId add_publication(const DDS::DomainId_t domain_id,
                                 const RepoId& participant_id,
                                 const RepoId& topic_id,
                                 DataWriterCallbacks* publication,
                                 const DDS::DataWriterQos& qos,
                                 const TransportLocatorSeq& trans_info,
                                 const DDS::PublisherQos& publisher_qos) = 0;

  virtual bool update_publication_qos(const DDS::DomainId_t domain_id,
                                      const RepoId& participant_id,
                                      const RepoId& data_writer_id,
                                      const DDS::DataWriterQos& qos,
                                      const DDS::PublisherQos& publisher_qos) = 0;

  virtual bool remove_publication(const DDS::DomainId_t domain_id,
                                  const RepoId& participant_id,
                                  const RepoId& publication_id) = 0;

  // Subscriptions
  // TODO(jrw972): Why?
  virtual void pre_reader(DataReaderImpl* data_reader) = 0;

  virtual RepoId add_subscription(const DDS::DomainId_t domain_id,
                                  const RepoId& participant_id,
                                  const RepoId& topic_id,
                                  DataReaderCallbacks* subscription,
                                  const DDS::DataReaderQos& qos,
                                  const TransportLocatorSeq& trans_info,
                                  const DDS::SubscriberQos& subscriber_qos,
                                  const char* filter_class_name,
                                  const char* filter_expression,
                                  const DDS::StringSeq& expr_params) = 0;

  virtual bool update_subscription_qos(const DDS::DomainId_t domain_id,
                                       const RepoId& participant_id,
                                       const RepoId& data_reader_id,
                                       const DDS::DataReaderQos& qos,
                                       const DDS::SubscriberQos& subscriber_qos) = 0;

  virtual bool update_subscription_params(const DDS::DomainId_t domain_id,
                                          const RepoId& participant_id,
                                          const RepoId& subscription_id,
                                          const DDS::StringSeq& params) = 0;

  virtual bool remove_subscription(const DDS::DomainId_t domain_id,
                                   const RepoId& participant_id,
                                   const RepoId& subscription_id) = 0;

  // Associations
  // TODO(jrw972): Document why this method exists.
  virtual void association_complete(const DDS::DomainId_t domain_id,
                                    const RepoId& participant_id,
                                    const RepoId& local_id,
                                    const RepoId& remote_id) = 0;
};

class LegacyDiscovery : public DiscoveryI {
public:
  LegacyDiscovery(Discovery_rch discovery) :
    discovery_(discovery) {}

  // Participant
  RepoId generate_participant_guid() override;

  AddDomainStatus add_domain_participant(const DDS::DomainId_t domain_id,
                                         const DDS::DomainParticipantQos& qos) override;

#if defined(OPENDDS_SECURITY)
  AddDomainStatus add_domain_participant_secure(const DDS::DomainId_t domain_id,
						const DDS::DomainParticipantQos& qos,
						const OpenDDS::DCPS::RepoId& guid,
						DDS::Security::IdentityHandle id,
						DDS::Security::PermissionsHandle perm,
						DDS::Security::ParticipantCryptoHandle part_crypto) override;
#endif

  bool update_domain_participant_qos(const DDS::DomainId_t domain_id,
                                     const RepoId& participant_id,
				     const DDS::DomainParticipantQos& qos) override;

  bool remove_domain_participant(const DDS::DomainId_t domain_id,
                                 const RepoId& participant_id) override;

  bool ignore_domain_participant(const DDS::DomainId_t domain_id,
                                 const RepoId& participant_id,
				 const RepoId& ignore_id) override;

  bool ignore_topic(const DDS::DomainId_t domain_id,
                    const RepoId& participant_id,
		    const RepoId& ignore_id) override;

  bool ignore_publication(const DDS::DomainId_t domain_id,
                          const RepoId& participant_id,
			  const RepoId& ignore_id) override;

  bool ignore_subscription(const DDS::DomainId_t domain_id,
                           const RepoId& participant_id,
			   const RepoId& ignore_id) override;

  bool supports_liveliness() const override;

  void signal_liveliness(const DDS::DomainId_t domain_id,
                         const RepoId& participant_id,
			 DDS::LivelinessQosPolicyKind kind) override;

  RepoId bit_key_to_repo_id(DomainParticipantImpl* participant,
			    const char* bit_topic_name,
			    const DDS::BuiltinTopicKey_t& key) const override;

  DDS::Subscriber_ptr init_bit(DomainParticipantImpl* participant,
                               Domain* domain) override;

  void fini_bit(DCPS::DomainParticipantImpl* participant) override;

  // Topics
  TopicStatus assert_topic(const DDS::DomainId_t domain_id,
                           RepoId_out topic_id,
			   const RepoId& participant_id,
			   const char* topic_name,
			   const char* data_type_name,
			   const DDS::TopicQos& qos,
			   bool has_dcps_key,
			   TopicCallbacks* topic_callbacks) override;

  TopicStatus find_topic(const DDS::DomainId_t domain_id,
                         const RepoId& participant_id,
			 const char* topic_name,
			 CORBA::String_out data_type_name,
			 DDS::TopicQos_out qos,
			 RepoId_out topic_id) override;

  bool update_topic_qos(const DDS::DomainId_t domain_id,
                        const RepoId& topic_id,
			const RepoId& participant_id,
			const DDS::TopicQos& qos) override;

  TopicStatus remove_topic(const DDS::DomainId_t domain_id,
                           const RepoId& participant_id,
			   const RepoId& topic_id) override;

  // Publications
  void pre_writer(DataWriterImpl* data_writer) override;

  RepoId add_publication(const DDS::DomainId_t domain_id,
                         const RepoId& participant_id,
			 const RepoId& topic_id,
			 DataWriterCallbacks* publication,
			 const DDS::DataWriterQos& qos,
			 const TransportLocatorSeq& trans_info,
			 const DDS::PublisherQos& publisher_qos) override;

  bool update_publication_qos(const DDS::DomainId_t domain_id,
                              const RepoId& participant_id,
			      const RepoId& data_writer_id,
			      const DDS::DataWriterQos& qos,
			      const DDS::PublisherQos& publisher_qos) override;

  bool remove_publication(const DDS::DomainId_t domain_id,
                          const RepoId& participant_id,
			  const RepoId& publication_id) override;

  // Subscriptions
  void pre_reader(DataReaderImpl* data_reader) override;

  RepoId add_subscription(const DDS::DomainId_t domain_id,
                          const RepoId& participant_id,
			  const RepoId& topic_id,
			  DataReaderCallbacks* subscription,
			  const DDS::DataReaderQos& qos,
			  const TransportLocatorSeq& trans_info,
			  const DDS::SubscriberQos& subscriber_qos,
			  const char* filter_class_name,
			  const char* filter_expression,
			  const DDS::StringSeq& expr_params) override;

  bool update_subscription_qos(const DDS::DomainId_t domain_id,
                               const RepoId& participant_id,
			       const RepoId& data_reader_id,
			       const DDS::DataReaderQos& qos,
			       const DDS::SubscriberQos& subscriber_qos) override;

  bool update_subscription_params(const DDS::DomainId_t domain_id,
                                  const RepoId& participant_id,
				  const RepoId& subscription_id,
				  const DDS::StringSeq& params) override;

  bool remove_subscription(const DDS::DomainId_t domain_id,
                           const RepoId& participant_id,
			   const RepoId& subscription_id) override;

  // Associations
  void association_complete(const DDS::DomainId_t domain_id,
                            const RepoId& participant_id,
			    const RepoId& local_id,
			    const RepoId& remote_id) override;

private:
  Discovery_rch discovery_;
};

class Domain {
public:
 Domain(const DDS::DomainId_t domain_id,
	DiscoveryI* discovery) :
   domain_id_(domain_id),
   discovery_(discovery)
  {}

  ~Domain() {
    delete discovery_;
  }

  DDS::DomainId_t domain_id() const { return domain_id_; }

  // Participant
  RepoId generate_participant_guid();

  AddDomainStatus add_domain_participant(const DDS::DomainParticipantQos& qos);

#if defined(OPENDDS_SECURITY)
  AddDomainStatus add_domain_participant_secure(
						const DDS::DomainParticipantQos& qos,
						const OpenDDS::DCPS::RepoId& guid,
						DDS::Security::IdentityHandle id,
						DDS::Security::PermissionsHandle perm,
						DDS::Security::ParticipantCryptoHandle part_crypto);
#endif

  bool update_domain_participant_qos(const RepoId& participant_id,
				     const DDS::DomainParticipantQos& qos);

  bool remove_domain_participant(const RepoId& participant_id);

  bool ignore_domain_participant(const RepoId& participant_id,
				 const RepoId& ignore_id);

  bool ignore_topic(const RepoId& participant_id,
		    const RepoId& ignore_id);

  bool ignore_publication(const RepoId& participant_id,
			  const RepoId& ignore_id);

  bool ignore_subscription(const RepoId& participant_id,
			   const RepoId& ignore_id);

  bool supports_liveliness() const;

  void signal_liveliness(const RepoId& participant_id,
			 DDS::LivelinessQosPolicyKind kind);

  RepoId bit_key_to_repo_id(DomainParticipantImpl* participant,
			    const char* bit_topic_name,
			    const DDS::BuiltinTopicKey_t& key) const;

  DDS::Subscriber_ptr init_bit(DomainParticipantImpl* participant);

  void fini_bit(DCPS::DomainParticipantImpl* participant);

  // Topics
  TopicStatus assert_topic(RepoId_out topic_id,
			   const RepoId& participant_id,
			   const char* topic_name,
			   const char* data_type_name,
			   const DDS::TopicQos& qos,
			   bool has_dcps_key,
			   TopicCallbacks* topic_callbacks);

  TopicStatus find_topic(const RepoId& participant_id,
			 const char* topic_name,
			 CORBA::String_out data_type_name,
			 DDS::TopicQos_out qos,
			 RepoId_out topic_id);

  bool update_topic_qos(const RepoId& topic_id,
			const RepoId& participant_id,
			const DDS::TopicQos& qos);

  TopicStatus remove_topic(const RepoId& participant_id,
			   const RepoId& topic_id);

  // Publications
  void pre_writer(DataWriterImpl* data_writer);

  RepoId add_publication(const RepoId& participant_id,
			 const RepoId& topic_id,
			 DataWriterCallbacks* publication,
			 const DDS::DataWriterQos& qos,
			 const TransportLocatorSeq& trans_info,
			 const DDS::PublisherQos& publisher_qos);

  bool update_publication_qos(const RepoId& participant_id,
			      const RepoId& data_writer_id,
			      const DDS::DataWriterQos& qos,
			      const DDS::PublisherQos& publisher_qos);

  bool remove_publication(const RepoId& participant_id,
			  const RepoId& publication_id);

  // Subscriptions
  void pre_reader(DataReaderImpl* data_reader);

  RepoId add_subscription(const RepoId& participant_id,
			  const RepoId& topic_id,
			  DataReaderCallbacks* subscription,
			  const DDS::DataReaderQos& qos,
			  const TransportLocatorSeq& trans_info,
			  const DDS::SubscriberQos& subscriber_qos,
			  const char* filter_class_name,
			  const char* filter_expression,
			  const DDS::StringSeq& expr_params);

  bool update_subscription_qos(const RepoId& participant_id,
			       const RepoId& data_reader_id,
			       const DDS::DataReaderQos& qos,
			       const DDS::SubscriberQos& subscriber_qos);

  bool update_subscription_params(const RepoId& participant_id,
				  const RepoId& subscription_id,
				  const DDS::StringSeq& params);

  bool remove_subscription(const RepoId& participant_id,
			   const RepoId& subscription_id);

  // Associations
  void association_complete(const RepoId& participant_id,
			    const RepoId& local_id,
			    const RepoId& remote_id);

private:
  const DDS::DomainId_t domain_id_;
  DiscoveryI* discovery_;
};

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#endif /* OPENDDS_DCPS_DOMAIN_H  */
