/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef OPENDDS_DCPS_INSTANCESTATE_H
#define OPENDDS_DCPS_INSTANCESTATE_H

#include "Definitions.h"
#include "GuidUtils.h"
#include "PoolAllocator.h"
#include "SporadicTask.h"
#include "TimeTypes.h"
#include "dcps_export.h"

#include <dds/DdsDcpsInfrastructureC.h>

#include <ace/Time_Value.h>

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

class DataReaderImpl;
typedef RcHandle<DataReaderImpl> DataReaderImpl_rch;
typedef WeakRcHandle<DataReaderImpl> DataReaderImpl_wrch;

class InstanceState;
typedef RcHandle<InstanceState> InstanceState_rch;

class ReceivedDataElement;

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
class OpenDDS_Dcps_Export InstanceState : public RcObject {
public:
  InstanceState(const DataReaderImpl_rch& reader,
                ACE_Recursive_Thread_Mutex& lock,
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

  RepoIdSet::const_iterator writers_begin() const { return writers_.begin(); }
  RepoIdSet::const_iterator writers_end() const { return writers_.end(); }

  /// DISPOSE message received for this instance.
  /// Return flag indicates whether the instance state was changed.
  /// This flag is used by concrete DataReader to determine whether
  /// it should notify listener. If state is not changed, the dispose
  /// message is ignored.
  bool dispose_was_received(const GUID_t& writer_id);

  /// UNREGISTER message received for this instance.
  /// Return flag indicates whether the instance state was changed.
  /// This flag is used by concrete DataReader to determine whether
  /// it should notify listener. If state is not changed, the unregister
  /// message is ignored.
  bool unregister_was_received(const GUID_t& writer_id);

  /// Data sample received for this instance.
  void data_was_received(const GUID_t& writer_id);

  /// LIVELINESS message received for this DataWriter.
  void lively(const GUID_t& writer_id);

  /// A read or take operation has been performed on this instance.
  void accessed();

  bool most_recent_generation(ReceivedDataElement* item) const;

  /// DataReader has become empty.  Returns true if the instance was released.
  bool empty(bool value);

  /// Schedule a pending release of resources.
  void schedule_pending();

  /// Schedule an immediate release of resources.
  void schedule_release();

  /// Cancel a scheduled or pending release of resources.
  void cancel_release();

  /// Remove the instance if it's instance has no samples
  /// and no writers.
  /// Returns true if the instance was released.
  bool release_if_empty();

  /// Remove the instance immediately.
  void release();

  /// Returns true if the writer is a writer of this instance.
  bool writes_instance(const GUID_t& writer_id) const
  {
    ACE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, guard, lock_, false);
    return writers_.count(writer_id);
  }

  WeakRcHandle<DataReaderImpl> data_reader() const;
  void state_updated() const;

  void set_owner (const GUID_t& owner);
  GUID_t get_owner ();
  bool is_exclusive () const;
  bool registered();
  void registered (bool flag);
  bool is_last (const GUID_t& pub);

  bool no_writer () const;

  void reset_ownership (DDS::InstanceHandle_t instance);

  DDS::InstanceHandle_t instance_handle() const { return handle_; }

  /// Return string of the name of the current instance state
  const char* instance_state_string() const;

  /// Return string of the name of the instance state kind passed
  static const char* instance_state_string(DDS::InstanceStateKind value);

  /// Return string representation of the instance state mask passed
  static OPENDDS_STRING instance_state_mask_string(DDS::InstanceStateMask mask);

private:
  bool reactor_is_shut_down() const;

  ACE_Recursive_Thread_Mutex& lock_;
  ACE_Thread_Mutex owner_lock_;

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

  /// Number of times the instance state changes
  /// from NOT_ALIVE_DISPOSED to ALIVE.
  size_t disposed_generation_count_;

  /// Number of times the instance state changes
  /// from NOT_ALIVE_NO_WRITERS to ALIVE.
  size_t no_writers_generation_count_;

  /**
   * Keep track of whether the DataReader is empty or not.
   */
  bool empty_;

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
  DDS::InstanceHandle_t handle_;

  RepoIdSet writers_;
  GUID_t owner_;
  bool exclusive_;
  /// registered with participant so it can be called back as
  /// the owner is updated.
  bool registered_;

  RcHandle<SporadicTask> release_task_;

  void do_release(const MonotonicTimePoint& now);
};

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#if defined (__ACE_INLINE__)
# include "InstanceState.inl"
#endif  /* __ACE_INLINE__ */

#endif /* OPENDDS_DCPS_INSTANCESTATE_H */
