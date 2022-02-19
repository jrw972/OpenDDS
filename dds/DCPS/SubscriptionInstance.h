/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef OPENDDS_DCPS_SUBSCRIPTION_INSTANCE_H
#define OPENDDS_DCPS_SUBSCRIPTION_INSTANCE_H

#include "dcps_export.h"
#include "ReceivedDataElementList.h"
#include "ReceivedDataStrategy.h"
#include "InstanceState.h"
#include "RcObject.h"
#include "RakeResults_T.h"
#include "Observer.h"
#include "ValueWriter.h"
#include "GroupRakeData.h"

#include "dds/DdsDcpsInfrastructureC.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

class DataReaderImpl;
class GroupRakeData;

OpenDDS_Dcps_Export inline
void sample_info(DDS::SampleInfo& sample_info,
                 const ReceivedDataElement* ptr)
{
  sample_info.sample_rank = 0;

  // generation_rank =
  //    (MRSIC.disposed_generation_count +
  //     MRSIC.no_writers_generation_count)
  //  - (S.disposed_generation_count +
  //     S.no_writers_generation_count)
  //
  sample_info.generation_rank =
      (sample_info.disposed_generation_count +
          sample_info.no_writers_generation_count) -
          sample_info.generation_rank;

  // absolute_generation_rank =
  //     (MRS.disposed_generation_count +
  //      MRS.no_writers_generation_count)
  //   - (S.disposed_generation_count +
  //      S.no_writers_generation_count)
  //
  sample_info.absolute_generation_rank =
      (static_cast<CORBA::Long>(ptr->disposed_generation_count_) +
          static_cast<CORBA::Long>(ptr->no_writers_generation_count_)) -
          sample_info.absolute_generation_rank;

  sample_info.opendds_reserved_publication_seq = ptr->sequence_.getValue();
}


/**
  * @class SubscriptionInstance
  *
  * @brief Struct that has information about an instance and the instance
  *        sample list.
  */
class OpenDDS_Dcps_Export SubscriptionInstance : public RcObject {
public:
  SubscriptionInstance(DataReaderImpl* reader,
                       const DDS::DataReaderQos& qos,
                       DDS::InstanceHandle_t handle,
                       bool owns_handle);

  ~SubscriptionInstance();

  DDS::InstanceHandle_t instance_handle() const
  {
    return instance_handle_;
  }

  long deadline_timer_id() const
  {
    return deadline_timer_id_;
  }

  void deadline_timer_id(long dti)
  {
    deadline_timer_id_ = dti;
  }

  MonotonicTimePoint last_accepted() const
  {
    return last_accepted_;
  }

  void last_accepted(const MonotonicTimePoint& mtp)
  {
    last_accepted_ = mtp;
  }

  void set_last_accepted()
  {
    last_accepted_.set_to_now();
  }

  void accessed(InstanceStateUpdateList& isul)
  {
    instance_state_->accessed(isul);
  }

  CORBA::ULong combined_state() const
  {
    return instance_state_->combined_state();
  }

  void lively(const PublicationId& writer_id,
              InstanceStateUpdateList& isul)
  {
    instance_state_->lively(writer_id, isul);
  }

  void data_was_received(const PublicationId& writer_id,
                         InstanceStateUpdateList& isul)
  {
    return instance_state_->data_was_received(writer_id, isul);
  }

  bool dispose_was_received(const PublicationId& writer_id,
                            InstanceStateUpdateList& isul)
  {
    return instance_state_->dispose_was_received(writer_id, isul);
  }

  bool unregister_was_received(const PublicationId& writer_id,
                               InstanceStateUpdateList& isul)
  {
    return instance_state_->unregister_was_received(writer_id, isul);
  }

  size_t disposed_generation_count() const
  {
    return instance_state_->disposed_generation_count();
  }

  size_t no_writers_generation_count() const
  {
    return instance_state_->no_writers_generation_count();
  }

  bool writes_instance(const PublicationId& writer_id) const
  {
    return instance_state_->writes_instance(writer_id);
  }

  bool is_last(const PublicationId& writer_id)
  {
    return instance_state_->is_last(writer_id);
  }

  PublicationId& get_owner ()
  {
    return instance_state_->get_owner();
  }

  WeakRcHandle<DataReaderImpl> data_reader() const
  {
    return instance_state_->data_reader();
  }

  bool registered()
  {
    return instance_state_->registered();
  }

  void registered (bool flag)
  {
    instance_state_->registered(flag);
  }

  void reset_ownership (DDS::InstanceHandle_t instance)
  {
    instance_state_->reset_ownership(instance);
  }

  void set_owner (const PublicationId& owner)
  {
    instance_state_->set_owner(owner);
  }

  bool is_exclusive () const
  {
    return instance_state_->is_exclusive();
  }

  DDS::InstanceStateKind instance_state() const
  {
    return instance_state_->instance_state();
  }

  void cancel_release()
  {
    instance_state_->cancel_release();
  }

  CORBA::ULong sample_count() const
  {
    return rcvd_samples_.size_;
  }

  DDS::SampleStateKind head_sample_state() const
  {
    return rcvd_samples_.head_->sample_state_;
  }

  void discard_oldest_sample(InstanceStateUpdateList& isul)
  {
    // Discard the oldest previously-read sample
    OpenDDS::DCPS::ReceivedDataElement *item = rcvd_samples_.head_;
    remove(item, isul);
    item->dec_ref();
  }

  OpenDDS::DCPS::ReceivedDataElement* remove_head(InstanceStateUpdateList& isul)
  {
    OpenDDS::DCPS::ReceivedDataElement* head_ptr = rcvd_samples_.head_;
    remove(head_ptr, isul);
    return head_ptr;
  }

  bool no_coherent_change() const
  {
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
    for (ReceivedDataElement *item = rcvd_samples_.head_;
         item != 0; item = item->next_data_sample_) {
      if (!item->coherent_change_) {
        return true;
      }
    }
    return false;
#else
    return true;
#endif
  }

  bool has_zero_copies() const
  {
    for (OpenDDS::DCPS::ReceivedDataElement *item = rcvd_samples_.head_;
         item != 0; item = item->next_data_sample_) {
      if (item->zero_copy_cnt_ > 0) {
        return true;
      }
    }

    return false;
  }

  void get_ordered_data(GroupRakeData& data,
                        GroupRakeData& group_coherent_ordered_data,
                        DDS::SampleStateMask sample_states, DDS::ViewStateMask view, DDS::InstanceStateMask inst)
  {
    if (instance_state_->match(view, inst)) {
      size_t i = 0;
      for (ReceivedDataElement* item = rcvd_samples_.head_; item != 0; item = item->next_data_sample_) {
        if ((item->sample_state_ & sample_states) && !item->coherent_change_) {
          data.insert_sample(item, rchandle_from(this), ++i);
          group_coherent_ordered_data.insert_sample(item, rchandle_from(this), ++i);
        }
      }
    }
  }

  template <typename MessageType>
  bool read_next_sample(MessageType& received_data,
                        DDS::SampleInfo& sample_info_ref,
                        Observer_rch observer,
                        const ValueWriterDispatcher* vwd,
                        DDS::DataReader* data_reader,
                        InstanceStateUpdateList& isul)
  {
    bool found_data = false;
    bool most_recent_generation = false;

    for (ReceivedDataElement* item = rcvd_samples_.head_; !found_data && item; item = item->next_data_sample_) {
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
      if (item->coherent_change_) continue;
#endif

      if (item->sample_state_ & DDS::NOT_READ_SAMPLE_STATE) {
        if (item->registered_data_) {
          received_data = *static_cast<MessageType*>(item->registered_data_);
        }
        instance_state_->sample_info(sample_info_ref, item);
        read(item, isul);

        if (observer && item->registered_data_ && vwd) {
          Observer::Sample s(sample_info_ref.instance_handle, sample_info_ref.instance_state, *item, *vwd);
          observer->on_sample_read(data_reader, s);
        }

        if (!most_recent_generation) {
          most_recent_generation = instance_state_->most_recent_generation(item);
        }
        found_data = true;
      }
    }

    if (found_data) {
      if (most_recent_generation) {
        instance_state_->accessed(isul);
      }
      // Get the sample_ranks, generation_ranks, and
      // absolute_generation_ranks for this info_seq
      sample_info(sample_info_ref, rcvd_samples_.tail_);
    }

    return found_data;
  }

  template <typename MessageType>
  bool take_next_sample(MessageType& received_data,
                        DDS::SampleInfo& sample_info_ref,
                        Observer_rch observer,
                        const ValueWriterDispatcher* vwd,
                        DDS::DataReader* data_reader,
                        InstanceStateUpdateList& isul)
  {
    bool found_data = false;
    bool most_recent_generation = false;

    OpenDDS::DCPS::ReceivedDataElement* tail = 0;
    OpenDDS::DCPS::ReceivedDataElement* next;
    OpenDDS::DCPS::ReceivedDataElement* item = rcvd_samples_.head_;
    while (!found_data && item) {
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
      if (item->coherent_change_) {
        item = item->next_data_sample_;
        continue;
      }
#endif
      if (item->sample_state_ & DDS::NOT_READ_SAMPLE_STATE) {
        if (item->registered_data_) {
          received_data = *static_cast<MessageType*>(item->registered_data_);
        }
        instance_state_->sample_info(sample_info_ref, item);
        read(item, isul);

        if (observer && item->registered_data_ && vwd) {
          Observer::Sample s(sample_info_ref.instance_handle, sample_info_ref.instance_state, *item, *vwd);
          observer->on_sample_taken(data_reader, s);
        }

        if (!most_recent_generation) {
          most_recent_generation = instance_state_->most_recent_generation(item);
        }

        if (item == rcvd_samples_.tail_) {
          tail = rcvd_samples_.tail_;
          item = item->next_data_sample_;

        } else {
          next = item->next_data_sample_;

          remove(item, isul);
          item->dec_ref();

          item = next;
        }

        found_data = true;
      }
    }

    if (found_data) {
      if (most_recent_generation) {
        instance_state_->accessed(isul);
      }

      //
      // Get the sample_ranks, generation_ranks, and
      // absolute_generation_ranks for this info_seq
      //
      if (tail) {
        sample_info(sample_info_ref, tail);

        remove(tail, isul);
        tail->dec_ref();

      } else {
        sample_info(sample_info_ref, rcvd_samples_.tail_);
      }
    }

    return found_data;
  }

  template <typename MessageSequenceType>
  void read(DDS::SampleStateMask sample_states,
            RakeResults<MessageSequenceType>& results,
            Observer_rch observer,
            const ValueWriterDispatcher* vwd,
            DDS::DataReader* data_reader)
  {
    size_t i = 0;
    for (ReceivedDataElement* item = rcvd_samples_.head_; item; item = item->next_data_sample_) {
      if ((item->sample_state_ & sample_states)
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
          && !item->coherent_change_
#endif
          ) {
        results.insert_sample(item, rchandle_from(this), ++i);

        if (observer && item->registered_data_ && vwd) {
          Observer::Sample s(instance_handle_, instance_state_->instance_state(), *item, *vwd);
          observer->on_sample_read(data_reader, s);
        }
      }
    }
  }

  template <typename MessageSequenceType>
  void take(DDS::SampleStateMask sample_states,
            RakeResults<MessageSequenceType>& results,
            Observer_rch observer,
            const ValueWriterDispatcher* vwd,
            DDS::DataReader* data_reader)
  {
    size_t i = 0;
    for (ReceivedDataElement* item = rcvd_samples_.head_; item; item = item->next_data_sample_) {
      if ((item->sample_state_ & sample_states)
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
          && !item->coherent_change_
#endif
          ) {
        results.insert_sample(item, rchandle_from(this), ++i);

        if (observer && item->registered_data_ && vwd) {
          Observer::Sample s(instance_handle_, instance_state_->instance_state(), *item, *vwd);
          observer->on_sample_taken(data_reader, s);
        }
      }
    }
  }

  template <typename MessageSequenceType>
  void read_instance(DDS::SampleStateMask sample_states,
                     DDS::ViewStateMask view_states,
                     DDS::InstanceStateMask instance_states,
                     RakeResults<MessageSequenceType>& results,
                     Observer_rch observer,
                     const ValueWriterDispatcher* vwd,
                     DDS::DataReader* data_reader)
  {
    if (instance_state_->match(view_states, instance_states)) {
      size_t i = 0;
      for (ReceivedDataElement* item = rcvd_samples_.head_; item; item = item->next_data_sample_) {
        if ((item->sample_state_ & sample_states)
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
            && !item->coherent_change_
#endif
            ) {
          results.insert_sample(item, rchandle_from(this), ++i);
          if (observer && item->registered_data_ && vwd) {
            Observer::Sample s(instance_handle_, instance_state_->instance_state(), *item, *vwd);
            observer->on_sample_read(data_reader, s);
          }
        }
      }
    }
  }

  template <typename MessageSequenceType>
  void take_instance(DDS::SampleStateMask sample_states,
                     DDS::ViewStateMask view_states,
                     DDS::InstanceStateMask instance_states,
                     RakeResults<MessageSequenceType>& results,
                     Observer_rch observer,
                     const ValueWriterDispatcher* vwd,
                     DDS::DataReader* data_reader)
  {
    if (instance_state_->match(view_states, instance_states)) {
      size_t i = 0;
      for (ReceivedDataElement* item = rcvd_samples_.head_; item; item = item->next_data_sample_) {
        if ((item->sample_state_ & sample_states)
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
            && !item->coherent_change_
#endif
            ) {
          results.insert_sample(item, rchandle_from(this), ++i);
          if (observer && item->registered_data_ && vwd) {
            Observer::Sample s(instance_handle_, instance_state_->instance_state(), *item, *vwd);
            observer->on_sample_taken(data_reader, s);
          }
        }
      }
    }
  }

  template <typename MessageType>
  bool contains_sample_filtered(DDS::SampleStateMask sample_states,
                                bool filter_has_non_key_fields,
                                const OpenDDS::DCPS::FilterEvaluator& evaluator,
                                const DDS::StringSeq& params)
  {
    for (ReceivedDataElement* item = rcvd_samples_.head_; item != 0; item = item->next_data_sample_) {
      if ((item->sample_state_ & sample_states)
#ifndef OPENDDS_NO_OBJECT_MODEL_PROFILE
          && !item->coherent_change_
#endif
          && item->registered_data_) {
        if (!item->valid_data_ && filter_has_non_key_fields) {
          continue;
        }
        if (evaluator.eval(*static_cast<MessageType*>(item->registered_data_), params)) {
          return true;
        }
      }
    }

    return false;
  }

  typedef OPENDDS_MAP(SubscriptionInstance*, InstanceData) InstanceMap;
  typedef OPENDDS_SET(SubscriptionInstance*) InstanceSet;

  template <class FwdIter>
    void copy_into(CORBA::ULong idx,
                   DDS::SampleInfoSeq& info_seq,
                   FwdIter iter,
                   InstanceMap& inst_map,
                   InstanceSet& released_instances,
                   bool take,
                   InstanceStateUpdateList& isul)
  {
    ReceivedDataElement* rde = iter->rde_;

    // 2. Per-sample SampleInfo (not the three *_rank variables) and state
    instance_state_->sample_info(info_seq[idx], rde);
    read(rde, isul);

    // 3. Record some info about per-instance SampleInfo (*_rank) so that
    //    we can fill in the ranks after the loop has completed
    std::pair<typename InstanceMap::iterator, bool> result =
      inst_map.insert(std::make_pair(this, InstanceData()));
    InstanceData& id = result.first->second;

    if (result.second) { // first time we've seen this Instance
      ReceivedDataElement& mrs = *rcvd_samples_.tail_;
      id.MRS_disposed_gc_ =
        static_cast<CORBA::Long>(mrs.disposed_generation_count_);
      id.MRS_nowriters_gc_ =
        static_cast<CORBA::Long>(mrs.no_writers_generation_count_);
    }

    if (iter->index_in_instance_ >= id.MRSIC_index_) {
      id.MRSIC_index_ = iter->index_in_instance_;
      id.MRSIC_disposed_gc_ =
        static_cast<CORBA::Long>(rde->disposed_generation_count_);
      id.MRSIC_nowriters_gc_ =
        static_cast<CORBA::Long>(rde->no_writers_generation_count_);
    }

    if (!id.most_recent_generation_) {
      id.most_recent_generation_ =
        instance_state_->most_recent_generation(rde);
    }

    id.sampleinfo_positions_.push_back(idx);

    // 4. Take
    if (take) {
      // If removing the sample releases it
      if (remove(rde, isul)) {
        // Prevent access of the SampleInfo, below
        released_instances.insert(this);
      }
      rde->dec_ref();
    }
  }

  void add(ReceivedDataElement* ptr,
           InstanceStateUpdateList& isul)
  {
    rcvd_strategy_->add(ptr);
    instance_state_->inc_not_read_count(isul);
  }

  void accept_coherent(const PublicationId& writer,
                       const RepoId& publisher)
  {
    rcvd_strategy_->accept_coherent(writer, publisher);
  }

  void reject_coherent(const PublicationId& writer,
                       const RepoId& publisher)
  {
    rcvd_strategy_->reject_coherent(writer, publisher);
  }

  void set_current_sample_time()
  {
    last_sample_tv_ = cur_sample_tv_;
    cur_sample_tv_.set_to_now();
  }

  const MonotonicTimePoint& current_sample_time()
  {
    return cur_sample_tv_;
  }

  const MonotonicTimePoint& last_sample_time()
  {
    return last_sample_tv_;
  }

  void last_sequence(const SequenceNumber& sn)
  {
    last_sequence_ = sn;
  }

  void purge_data()
  {
    while (rcvd_samples_.size_ > 0) {
      OpenDDS::DCPS::ReceivedDataElement* head = rcvd_samples_.head_;
      rcvd_samples_.remove(head);
      head->dec_ref();
    }
  }

private:
  /// Sequence number of the most recent data sample received
  SequenceNumber last_sequence_;

  /// ReceivedDataElementList strategy
  unique_ptr<ReceivedDataStrategy> rcvd_strategy_;

  /// The instance handle for the registered object
  const DDS::InstanceHandle_t instance_handle_;

  const bool owns_handle_;

  MonotonicTimePoint last_sample_tv_;

  MonotonicTimePoint cur_sample_tv_;

  long deadline_timer_id_;

  MonotonicTimePoint last_accepted_;

  /// Instance state for this instance
  const InstanceState_rch instance_state_;

  /// Data sample(s) in this instance
  ReceivedDataElementList rcvd_samples_;

  void read(ReceivedDataElement* rde,
            InstanceStateUpdateList& isul)
  {
    if (rde->sample_state_ == DDS::NOT_READ_SAMPLE_STATE) {
      rde->sample_state_ = DDS::READ_SAMPLE_STATE;
      instance_state_->inc_read_count(isul);
    }
  }

  bool remove(ReceivedDataElement* rde,
              InstanceStateUpdateList& isul)
  {
    if (rde->sample_state_ == DDS::READ_SAMPLE_STATE) {
      instance_state_->dec_read_count(isul);
    } else {
      instance_state_->dec_not_read_count(isul);
    }
    rcvd_samples_.remove(rde);
    if (rcvd_samples_.size_ == 0 && instance_state_->release_pending() && instance_state_->no_writer()) {
      isul.remove(instance_handle_);
      return true;
    }
    return false;
  }
};

typedef RcHandle<SubscriptionInstance> SubscriptionInstance_rch;

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#endif /* OPENDDS_DCPS_SUBSCRIPTION_INSTANCE_H */
