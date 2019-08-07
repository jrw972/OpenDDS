/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "DCPS/DdsDcps_pch.h" //Only the _pch include should start with DCPS/
#include "Domain.h"
#include "DomainParticipantImpl.h"

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

void Domain::association_complete(const DomainParticipantImpl* dp,
				  const RepoId& local_id,
				  const RepoId& remote_id) {
  discovery_->association_complete(domain_id_, dp->get_id(), local_id, remote_id);
}

bool Domain::update_publication_qos(const DomainParticipantImpl* dp,
				    const RepoId& data_writer_id,
				    const DDS::DataWriterQos& qos,
				    const DDS::PublisherQos& publisher_qos) {
  return discovery_->update_publication_qos(domain_id_, dp->get_id(), data_writer_id, qos, publisher_qos);
}

void Domain::pre_writer(DataWriterImpl* data_writer) {
  discovery_->pre_writer(data_writer);
}

RepoId Domain::add_publication(const DomainParticipantImpl* dp,
			       const RepoId& topic_id,
			       DataWriterCallbacks* publication,
			       const DDS::DataWriterQos& qos,
			       const TransportLocatorSeq& trans_info,
			       const DDS::PublisherQos& publisher_qos) {
  return discovery_->add_publication(domain_id_, dp->get_id(), topic_id, publication, qos, trans_info, publisher_qos);
}

bool Domain::supports_liveliness() const {
  return discovery_->supports_liveliness();
}

bool Domain::remove_publication(const DomainParticipantImpl* dp,
				const RepoId& publication_id) {
  return discovery_->remove_publication(domain_id_, dp->get_id(), publication_id);
}

bool Domain::update_subscription_qos(const DomainParticipantImpl* dp,
				     const RepoId& data_reader_id,
				     const DDS::DataReaderQos& qos,
				     const DDS::SubscriberQos& subscriber_qos) {
  return discovery_->update_subscription_qos(domain_id_, dp->get_id(), data_reader_id, qos, subscriber_qos);
}

void Domain::pre_reader(DataReaderImpl* data_reader) {
  discovery_->pre_reader(data_reader);
}

RepoId Domain::add_subscription(const DomainParticipantImpl* dp,
				const RepoId& topic_id,
				DataReaderCallbacks* subscription,
				const DDS::DataReaderQos& qos,
				const TransportLocatorSeq& trans_info,
				const DDS::SubscriberQos& subscriber_qos,
				const char* filter_class_name,
				const char* filter_expression,
				const DDS::StringSeq& expr_params) {
  return discovery_->add_subscription(domain_id_, dp->get_id(), topic_id, subscription, qos, trans_info, subscriber_qos, filter_class_name, filter_expression, expr_params);
}

RepoId Domain::bit_key_to_repo_id(const DomainParticipantImpl* dp,
				  const char* bit_topic_name,
				  const DDS::BuiltinTopicKey_t& key) const {
  return discovery_->bit_key_to_repo_id(dp, bit_topic_name, key);
}

bool Domain::update_subscription_params(const DomainParticipantImpl* dp,
					const RepoId& subscription_id,
					const DDS::StringSeq& params) {
  return discovery_->update_subscription_params(domain_id_, dp->get_id(), subscription_id, params);
}

bool Domain::remove_subscription(const DomainParticipantImpl* dp,
				 const RepoId& subscription_id) {
  return discovery_->remove_subscription(domain_id_, dp->get_id(), subscription_id);
}

bool Domain::remove_domain_participant(const DomainParticipantImpl* dp) {
  return discovery_->remove_domain_participant(domain_id_, dp->get_id());
}

bool Domain::update_topic_qos(const RepoId& topic_id,
                              const DomainParticipantImpl* dp,
			      const DDS::TopicQos& qos) {
  return discovery_->update_topic_qos(domain_id_, topic_id, dp->get_id(), qos);
}

TopicStatus Domain::assert_topic(RepoId_out topic_id,
                                 const DomainParticipantImpl* dp,
				 const char* topic_name,
				 const char* data_type_name,
				 const DDS::TopicQos& qos,
				 bool has_dcps_key,
				 TopicCallbacks* topic_callbacks) {
  return discovery_->assert_topic(domain_id_, topic_id, dp->get_id(), topic_name, data_type_name, qos, has_dcps_key, topic_callbacks);
}

TopicStatus Domain::remove_topic(const DomainParticipantImpl* dp,
				 const RepoId& topic_id) {
  return discovery_->remove_topic(domain_id_, dp->get_id(), topic_id);
}

TopicStatus Domain::find_topic(const DomainParticipantImpl* dp,
			       const char* topic_name,
			       CORBA::String_out data_type_name,
			       DDS::TopicQos_out qos,
			       RepoId_out topic_id) {
  return discovery_->find_topic(domain_id_, dp->get_id(), topic_name, data_type_name, qos, topic_id);
}

void Domain::fini_bit(DCPS::DomainParticipantImpl* participant) {
  discovery_->fini_bit(participant);
}

bool Domain::update_domain_participant_qos(const DomainParticipantImpl* dp) {
  return discovery_->update_domain_participant_qos(domain_id_, dp->get_id(), dp->qos());
}

bool Domain::ignore_domain_participant(const DomainParticipantImpl* dp,
				       const RepoId& ignore_id) {
  return discovery_->ignore_domain_participant(domain_id_, dp->get_id(), ignore_id);
}

bool Domain::ignore_topic(const DomainParticipantImpl* dp,
			  const RepoId& ignore_id) {
  return discovery_->ignore_topic(domain_id_, dp->get_id(), ignore_id);
}

bool Domain::ignore_publication(const DomainParticipantImpl* dp,
				const RepoId& ignore_id) {
  return discovery_->ignore_publication(domain_id_, dp->get_id(), ignore_id);
}

bool Domain::ignore_subscription(const DomainParticipantImpl* dp,
				 const RepoId& ignore_id) {
  return discovery_->ignore_subscription(domain_id_, dp->get_id(), ignore_id);
}

DDS::ReturnCode_t Domain::add_domain_participant(DomainParticipantImpl* dp) {
  return discovery_->add_domain_participant(domain_id_, dp);
}

DDS::Subscriber_ptr Domain::init_bit(DomainParticipantImpl* participant) {
  return discovery_->init_bit(participant, this);
}

void Domain::signal_liveliness(const DomainParticipantImpl* dp,
			       DDS::LivelinessQosPolicyKind kind) {
  discovery_->signal_liveliness(domain_id_, dp->get_id(), kind);
}

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
