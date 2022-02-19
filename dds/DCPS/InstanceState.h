/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef OPENDDS_DCPS_INSTANCESTATE_H
#define OPENDDS_DCPS_INSTANCESTATE_H

#include "dcps_export.h"
#include "ace/Time_Value.h"
#include "dds/DdsDcpsInfrastructureC.h"
#include "Definitions.h"
#include "GuidUtils.h"
#include "PoolAllocator.h"
#include "ReactorInterceptor.h"
#include "RepoIdTypes.h"
#include "TimeTypes.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

class DataReaderImpl;
class ReceivedDataElement;

class InstanceState;
typedef RcHandle<InstanceState> InstanceState_rch;

// data structures used by copy_into()
typedef OPENDDS_VECTOR(CORBA::ULong) IndexList;

struct InstanceData {
  bool most_recent_generation_;
  size_t MRSIC_index_;
  IndexList sampleinfo_positions_;
  CORBA::Long MRSIC_disposed_gc_;
  CORBA::Long MRSIC_nowriters_gc_;
  CORBA::Long MRS_disposed_gc_;
  CORBA::Long MRS_nowriters_gc_;

  InstanceData()
    : most_recent_generation_(false)
    , MRSIC_index_(0)
    , MRSIC_disposed_gc_(0)
    , MRSIC_nowriters_gc_(0)
    , MRS_disposed_gc_(0)
    , MRS_nowriters_gc_(0) {}
};

class InstanceStateUpdateList {
public:
  struct InstanceStateUpdate {
    DDS::InstanceHandle_t handle;
    CORBA::ULong previous_state;
    CORBA::ULong current_state;

    InstanceStateUpdate(DDS::InstanceHandle_t a_handle,
                        CORBA::ULong a_previous_state,
                        CORBA::ULong a_current_state)
      : handle(a_handle)
      , previous_state(a_previous_state)
      , current_state(a_current_state)
    {}
  };

  void add(DDS::InstanceHandle_t handle,
           CORBA::ULong previous_state,
           CORBA::ULong current_state)
  {
    if (current_state != previous_state) {
      list_.push_back(InstanceStateUpdate(handle, previous_state, current_state));
    }
  }

  void remove(DDS::InstanceHandle_t handle)
  {
    set_.insert(handle);
  }

  typedef OPENDDS_VECTOR(InstanceStateUpdate) List;
  typedef List::const_iterator const_add_iterator;
  const_add_iterator add_begin() const { return list_.begin(); }
  const_add_iterator add_end() const { return list_.end(); }

  typedef OPENDDS_SET(DDS::InstanceHandle_t) Set;
  typedef Set::const_iterator const_remove_iterator;
  const_remove_iterator remove_begin() const { return set_.begin(); }
  const_remove_iterator remove_end() const { return set_.end(); }

private:
  List list_;
  Set set_;
};

/**
 * @class InstanceState
 *
 * @brief manage the states of a received data instance.
 *
 * Provide a mechanism to manage the view state and instance
 * state values for an instance contained within a DataReader.
 * The instance_state and view_state are managed by this class.
 * Accessors are provided to query the current value of each of
 * these states.
 */
class OpenDDS_Dcps_Export InstanceState : public ReactorInterceptor {
public:
  InstanceState(DataReaderImpl* reader,
                DDS::InstanceHandle_t handle);

  virtual ~InstanceState();

  /// Populate the SampleInfo structure
  void sample_info(DDS::SampleInfo& si,
                   const ReceivedDataElement* de);

  /// Access instance state.
  DDS::InstanceStateKind instance_state() const;

  /// Access view state.
  DDS::ViewStateKind view_state() const;

  bool match(DDS::ViewStateMask view, DDS::InstanceStateMask inst) const;

  /// Access disposed generation count
  size_t disposed_generation_count() const;

  /// Access no writers generation count
  size_t no_writers_generation_count() const;

  /// DISPOSE message received for this instance.
  /// Return flag indicates whether the instance state was changed.
  /// This flag is used by concrete DataReader to determine whether
  /// it should notify listener. If state is not changed, the dispose
  /// message is ignored.
  bool dispose_was_received(const PublicationId& writer_id,
                            InstanceStateUpdateList& isul);

  /// UNREGISTER message received for this instance.
  /// Return flag indicates whether the instance state was changed.
  /// This flag is used by concrete DataReader to determine whether
  /// it should notify listener. If state is not changed, the unregister
  /// message is ignored.
  bool unregister_was_received(const PublicationId& writer_id,
                               InstanceStateUpdateList& isul);

  /// Data sample received for this instance.
  void data_was_received(const PublicationId& writer_id,
                         InstanceStateUpdateList& isul);

  /// LIVELINESS message received for this DataWriter.
  void lively(const PublicationId& writer_id,
              InstanceStateUpdateList& isul);

  /// A read or take operation has been performed on this instance.
  void accessed(InstanceStateUpdateList& isul);

  bool most_recent_generation(ReceivedDataElement* item) const;

  /// Schedule an immediate release of resources.
  void schedule_release();

  /// Cancel a scheduled or pending release of resources.
  void cancel_release();

  /// Returns true if the writer is a writer of this instance.
  bool writes_instance(const PublicationId& writer_id) const
  {
    return writers_.count(writer_id);
  }

  WeakRcHandle<DataReaderImpl> data_reader() const;

  virtual int handle_timeout(const ACE_Time_Value& current_time,
                             const void* arg);

  void set_owner (const PublicationId& owner);
  PublicationId& get_owner ();
  bool is_exclusive () const;
  bool registered();
  void registered (bool flag);
  bool is_last (const PublicationId& pub);

  bool no_writer () const;

  void reset_ownership (DDS::InstanceHandle_t instance);

  DDS::InstanceHandle_t instance_handle() const { return handle_; }

  /// Return string of the name of the current instance state
  const char* instance_state_string() const;

  /// Return string of the name of the instance state kind passed
  static const char* instance_state_string(DDS::InstanceStateKind value);

  /// Return string representation of the instance state mask passed
  static OPENDDS_STRING instance_state_mask_string(DDS::InstanceStateMask mask);

  static inline CORBA::ULong combine_state(DDS::SampleStateMask sample_states,
                                           DDS::ViewStateMask view_states,
                                           DDS::InstanceStateMask instance_states)
  {
    return (sample_states << 5) | (view_states << 3) | instance_states;
  }

  CORBA::ULong combined_state() const
  {
    return combined_state_i();
  }

  void inc_not_read_count(InstanceStateUpdateList& isul)
  {
    const CORBA::ULong previous_state = combined_state_i();
    ++not_read_count_;
    isul.add(handle_, previous_state, combined_state_i());
  }

  void inc_read_count(InstanceStateUpdateList& isul)
  {
    const CORBA::ULong previous_state = combined_state_i();
    --not_read_count_;
    ++read_count_;
    isul.add(handle_, previous_state, combined_state_i());
  }

  void dec_not_read_count(InstanceStateUpdateList& isul)
  {
    const CORBA::ULong previous_state = combined_state_i();
    --not_read_count_;
    isul.add(handle_, previous_state, combined_state_i());
  }

  void dec_read_count(InstanceStateUpdateList& isul)
  {
    const CORBA::ULong previous_state = combined_state_i();
    --read_count_;
    isul.add(handle_, previous_state, combined_state_i());
  }

  bool release_pending() const
  {
    return release_pending_;
  }

private:
  bool reactor_is_shut_down() const;
  CORBA::ULong combined_state_i() const
  {
    const DDS::SampleStateKind sample_states =
      (not_read_count_ ? DDS::NOT_READ_SAMPLE_STATE : 0) |
      (read_count_ ? DDS::READ_SAMPLE_STATE : 0);
    return combine_state(sample_states, view_state_, instance_state_);
  }

  /**
   * Current instance state.
   *
   * Can have values defined as:
   *
   *   DDS::ALIVE_INSTANCE_STATE
   *   DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE
   *   DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
   *
   * and can be checked with the masks:
   *
   *   DDS::ANY_INSTANCE_STATE
   *   DDS::NOT_ALIVE_INSTANCE_STATE
   */
  DDS::InstanceStateKind instance_state_;

  /**
   * Current instance view state.
   *
   * Can have values defined as:
   *
   *   DDS::NEW_VIEW_STATE
   *   DDS::NOT_NEW_VIEW_STATE
   *
   * and can be checked with the mask:
   *
   *   DDS::ANY_VIEW_STATE
   */
  DDS::ViewStateKind view_state_;

  size_t not_read_count_;
  size_t read_count_;

  /// Number of times the instance state changes
  /// from NOT_ALIVE_DISPOSED to ALIVE.
  size_t disposed_generation_count_;

  /// Number of times the instance state changes
  /// from NOT_ALIVE_NO_WRITERS to ALIVE.
  size_t no_writers_generation_count_;

  /**
   * Keep track of whether the instance is waiting to be released.
   */
  bool release_pending_;

  /**
   * Keep track of a scheduled release timer.
   */
  long release_timer_id_;

  /**
   * Reference to our containing reader.  This is used to call back
   * and notify the reader that liveliness has been lost on this
   * instance.  It is also queried to determine if the DataReader is
   * empty -- that it contains no more sample data.
   */
  WeakRcHandle<DataReaderImpl> reader_;
  const DDS::InstanceHandle_t handle_;

  RepoIdSet writers_;
  PublicationId owner_;
  bool exclusive_;
  /// registered with participant so it can be called back as
  /// the owner is updated.
  bool registered_;

  struct CommandBase : Command {
    explicit CommandBase(InstanceState* instance_state)
      : instance_state_(instance_state)
    {}

    InstanceState* instance_state_;
  };

  struct CancelCommand : CommandBase {
    explicit CancelCommand(InstanceState* instance_state)
      : CommandBase(instance_state)
    {}

    void execute();
  };

  struct ScheduleCommand : CommandBase {
    ScheduleCommand(InstanceState* instance_state, const TimeDuration& delay)
      : CommandBase(instance_state)
      , delay_(delay)
    {}

    const TimeDuration delay_;
    void execute();
  };

};

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#if defined (__ACE_INLINE__)
# include "InstanceState.inl"
#endif  /* __ACE_INLINE__ */

#endif /* OPENDDS_DCPS_INSTANCESTATE_H */
