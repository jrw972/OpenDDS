/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef OPENDDS_DCPS_DOMAIN_H
#define OPENDDS_DCPS_DOMAIN_H

#include "Definitions.h"
#include "dds/DCPS/DiscoveryI.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

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
  DDS::ReturnCode_t add_domain_participant(DomainParticipantImpl* dp);

  bool update_domain_participant_qos(const DomainParticipantImpl* dp);

  bool remove_domain_participant(const DomainParticipantImpl* dp);

  bool ignore_domain_participant(const DomainParticipantImpl* dp,
				 const RepoId& ignore_id);

  bool ignore_topic(const DomainParticipantImpl* dp,
		    const RepoId& ignore_id);

  bool ignore_publication(const DomainParticipantImpl* dp,
			  const RepoId& ignore_id);

  bool ignore_subscription(const DomainParticipantImpl* dp,
			   const RepoId& ignore_id);

  bool supports_liveliness() const;

  void signal_liveliness(const DomainParticipantImpl* dp,
			 DDS::LivelinessQosPolicyKind kind);

  RepoId bit_key_to_repo_id(const DomainParticipantImpl* dp,
			    const char* bit_topic_name,
			    const DDS::BuiltinTopicKey_t& key) const;

  DDS::Subscriber_ptr init_bit(DomainParticipantImpl* participant);

  void fini_bit(DomainParticipantImpl* participant);

  // Topics
  TopicStatus assert_topic(RepoId_out topic_id,
			   const DomainParticipantImpl* dp,
			   const char* topic_name,
			   const char* data_type_name,
			   const DDS::TopicQos& qos,
			   bool has_dcps_key,
			   TopicCallbacks* topic_callbacks);

  TopicStatus find_topic(const DomainParticipantImpl* dp,
			 const char* topic_name,
			 CORBA::String_out data_type_name,
			 DDS::TopicQos_out qos,
			 RepoId_out topic_id);

  bool update_topic_qos(const RepoId& topic_id,
                        const DomainParticipantImpl* dp,
			const DDS::TopicQos& qos);

  TopicStatus remove_topic(const DomainParticipantImpl* dp,
			   const RepoId& topic_id);

  // Publications
  void pre_writer(DataWriterImpl* data_writer);

  RepoId add_publication(const DomainParticipantImpl* dp,
			 const RepoId& topic_id,
			 DataWriterCallbacks* publication,
			 const DDS::DataWriterQos& qos,
			 const TransportLocatorSeq& trans_info,
			 const DDS::PublisherQos& publisher_qos);

  bool update_publication_qos(const DomainParticipantImpl* dp,
			      const RepoId& data_writer_id,
			      const DDS::DataWriterQos& qos,
			      const DDS::PublisherQos& publisher_qos);

  bool remove_publication(const DomainParticipantImpl* dp,
			  const RepoId& publication_id);

  // Subscriptions
  void pre_reader(DataReaderImpl* data_reader);

  RepoId add_subscription(const DomainParticipantImpl* dp,
			  const RepoId& topic_id,
			  DataReaderCallbacks* subscription,
			  const DDS::DataReaderQos& qos,
			  const TransportLocatorSeq& trans_info,
			  const DDS::SubscriberQos& subscriber_qos,
			  const char* filter_class_name,
			  const char* filter_expression,
			  const DDS::StringSeq& expr_params);

  bool update_subscription_qos(const DomainParticipantImpl* dp,
			       const RepoId& data_reader_id,
			       const DDS::DataReaderQos& qos,
			       const DDS::SubscriberQos& subscriber_qos);

  bool update_subscription_params(const DomainParticipantImpl* dp,
				  const RepoId& subscription_id,
				  const DDS::StringSeq& params);

  bool remove_subscription(const DomainParticipantImpl* dp,
			   const RepoId& subscription_id);

  // Associations
  void association_complete(const DomainParticipantImpl* dp,
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
