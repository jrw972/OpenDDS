/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "UdpTransport.h"

#include "UdpInst_rch.h"
#include "UdpDataLink.h"
#include "UdpInst.h"
#include "UdpSendStrategy.h"
#include "UdpReceiveStrategy.h"

#include <dds/DCPS/LogAddr.h>
#include <dds/DCPS/NetworkResource.h>
#include <dds/DCPS/transport/framework/PriorityKey.h>
#include <dds/DCPS/transport/framework/TransportClient.h>
#include <dds/DCPS/transport/framework/TransportExceptions.h>
#include <dds/DCPS/AssociationData.h>

#include <ace/CDR_Base.h>
#include <ace/Log_Msg.h>

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

namespace {
  const Encoding::Kind encoding_kind = Encoding::KIND_UNALIGNED_CDR;
}

UdpTransport::UdpTransport(const UdpInst_rch& inst,
                           DDS::DomainId_t domain)
  : TransportImpl(inst, domain)
{
  if (!(configure_i(inst) && open())) {
    throw Transport::UnableToCreate();
  }
}

UdpInst_rch
UdpTransport::config() const
{
  return dynamic_rchandle_cast<UdpInst>(TransportImpl::config());
}


UdpDataLink_rch
UdpTransport::make_datalink(const ACE_INET_Addr& remote_address,
                            Priority priority, bool active)
{
  UdpDataLink_rch link(make_rch<UdpDataLink>(rchandle_from(this), priority, reactor_task(), active));
  // Configure link with transport configuration and reactor task:

  // Open logical connection:
  if (link->open(remote_address)) {
    return link;
  }

  ACE_ERROR((LM_ERROR,
              ACE_TEXT("(%P|%t) ERROR: ")
              ACE_TEXT("UdpTransport::make_datalink: ")
              ACE_TEXT("failed to open DataLink!\n")));

  return UdpDataLink_rch();
}

TransportImpl::AcceptConnectResult
UdpTransport::connect_datalink(const RemoteTransport& remote,
                               const ConnectionAttribs& attribs,
                               const TransportClient_rch& )
{
  UdpInst_rch cfg = config();
  if (!cfg && is_shut_down()) {
    return AcceptConnectResult(AcceptConnectResult::ACR_FAILED);
  }
  const ACE_INET_Addr remote_address = get_connection_addr(remote.blob_);
  const bool active = true;
  const PriorityKey key = blob_to_key(remote.blob_, attribs.priority_, cfg->send_receive_address(), active);

  VDBG_LVL((LM_DEBUG, "(%P|%t) UdpTransport::connect_datalink PriorityKey "
            "prio=%d, addr=%C, is_loopback=%d, is_active=%d\n",
            key.priority(), LogAddr(key.address()).c_str(), key.is_loopback(),
            key.is_active()), 2);

  GuardType guard(client_links_lock_);
  if (this->is_shut_down()) {
    return AcceptConnectResult(AcceptConnectResult::ACR_FAILED);
  }

  const UdpDataLinkMap::iterator it(client_links_.find(key));
  if (it != client_links_.end()) {
    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::connect_datalink found\n"));
    return AcceptConnectResult(UdpDataLink_rch(it->second));
  }

  // Create new DataLink for logical connection:
  UdpDataLink_rch link (make_datalink(remote_address,
                                       attribs.priority_,
                                       active));

  if (!link.is_nil()) {
    client_links_.insert(UdpDataLinkMap::value_type(key, link));
    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::connect_datalink connected\n"));
  }

  return AcceptConnectResult(link);
}

TransportImpl::AcceptConnectResult
UdpTransport::accept_datalink(const RemoteTransport& remote,
                              const ConnectionAttribs& attribs,
                              const TransportClient_rch& client)
{
  UdpInst_rch cfg = config();
  if (!cfg && is_shut_down()) {
    return AcceptConnectResult(AcceptConnectResult::ACR_FAILED);
  }
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connections_lock_);

  const PriorityKey key = blob_to_key(remote.blob_,
                                      attribs.priority_, cfg->send_receive_address(), false /* !active */);

  VDBG_LVL((LM_DEBUG, "(%P|%t) UdpTransport::accept_datalink PriorityKey "
            "prio=%d, addr=%C, is_loopback=%d, is_active=%d\n",
            key.priority(), LogAddr(key.address()).c_str(), key.is_loopback(),
            key.is_active()), 2);

  if (server_link_keys_.count(key)) {
    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::accept_datalink found\n"));
    return AcceptConnectResult(UdpDataLink_rch(server_link_));
  } else if (pending_server_link_keys_.count(key)) {
    pending_server_link_keys_.erase(key);
    server_link_keys_.insert(key);
    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::accept_datalink completed\n"));
    return AcceptConnectResult(UdpDataLink_rch(server_link_));
  } else {
    const DataLink::OnStartCallback callback(client, remote.repo_id_);
    pending_connections_[key].push_back(callback);
    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::accept_datalink pending\n"));
    return AcceptConnectResult(AcceptConnectResult::ACR_SUCCESS);
  }
}

void
UdpTransport::stop_accepting_or_connecting(const TransportClient_wrch& client,
                                           const GUID_t& remote_id,
                                           bool /*disassociate*/,
                                           bool /*association_failed*/)
{
  VDBG((LM_DEBUG, "(%P|%t) UdpTransport::stop_accepting_or_connecting\n"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connections_lock_);

  for (PendConnMap::iterator it = pending_connections_.begin();
       it != pending_connections_.end(); ++it) {
    for (Callbacks::iterator cit = it->second.begin(); cit != it->second.end(); ++cit) {
      if (cit->first == client && cit->second == remote_id) {
        it->second.erase(cit);
        break;
      }
    }
    if (it->second.empty()) {
      pending_connections_.erase(it);
      return;
    }
  }
}

bool
UdpTransport::configure_i(const UdpInst_rch& config)
{
  if (!config) {
    return false;
  }
  create_reactor_task(false, "UdpTransport" + config->name());

  // Our "server side" data link is created here, similar to the acceptor_
  // in the TcpTransport implementation.  This establishes a socket as an
  // endpoint that we can advertise to peers via connection_info_i().
  server_link_ = make_datalink(config->send_receive_address(), 0 /* priority */, false);
  return true;
}

void
UdpTransport::shutdown_i()
{
  // Shutdown reserved datalinks and release configuration:
  GuardType guard(client_links_lock_);
  for (UdpDataLinkMap::iterator it(client_links_.begin());
       it != client_links_.end(); ++it) {
    it->second->transport_shutdown();
  }
  client_links_.clear();

  if (server_link_) {
    server_link_->transport_shutdown();
    server_link_.reset();
  }
}

bool
UdpTransport::connection_info_i(TransportLocator& info, ConnectionInfoFlags flags) const
{
  UdpInst_rch cfg = config();
  if (cfg) {
    cfg->populate_locator(info, flags, domain_);
    return true;
  }
  return false;
}

ACE_INET_Addr
UdpTransport::get_connection_addr(const TransportBLOB& data) const
{
  ACE_INET_Addr local_address;
  NetworkResource network_resource;

  size_t len = data.length();
  const char* buffer = reinterpret_cast<const char*>(data.get_buffer());

  ACE_InputCDR cdr(buffer, len);
  if (cdr >> network_resource) {
    network_resource.to_addr(local_address);
  }

  return local_address;
}

void
UdpTransport::release_datalink(DataLink* link)
{
  GuardType guard(client_links_lock_);
  for (UdpDataLinkMap::iterator it(client_links_.begin());
       it != client_links_.end(); ++it) {
    // We are guaranteed to have exactly one matching DataLink
    // in the map; release any resources held and return.
    if (link == static_cast<DataLink*>(it->second.in())) {
      link->stop();
      client_links_.erase(it);
      return;
    }
  }
}

PriorityKey
UdpTransport::blob_to_key(const TransportBLOB& remote,
                          Priority priority,
                          ACE_INET_Addr local_addr,
                          bool active)
{
  NetworkResource network_resource;
  ACE_InputCDR cdr((const char*)remote.get_buffer(), remote.length());

  if (!(cdr >> network_resource)) {
    ACE_ERROR((LM_ERROR,
               ACE_TEXT("(%P|%t) ERROR: UdpTransport::blob_to_key")
               ACE_TEXT(" failed to de-serialize the NetworkResource\n")));
  }

  ACE_INET_Addr remote_address;
  network_resource.to_addr(remote_address);
  const bool is_loopback = remote_address == local_addr;

  return PriorityKey(priority, remote_address, is_loopback, active);
}

void
UdpTransport::passive_connection(const ACE_INET_Addr& remote_address,
                                 const ReceivedDataSample& data)
{
  UdpInst_rch cfg = config();
  if (!cfg) {
    return;
  }
  const size_t blob_len = data.data_length() - sizeof(Priority);
  Message_Block_Ptr payload(data.data());
  Priority priority;
  Serializer serializer(payload.get(), encoding_kind);
  serializer >> priority;
  TransportBLOB blob(static_cast<CORBA::ULong>(blob_len));
  blob.length(blob.maximum());
  serializer.read_octet_array(blob.get_buffer(), blob.length());

  // Send an ack so that the active side can return from
  // connect_datalink_i().  This is just a single byte of
  // arbitrary data, the remote side is not yet using the
  // framework (TransportHeader, DataSampleHeader,
  // ReceiveStrategy).
  const char ack_data = 23;
  if (server_link_->socket().send(&ack_data, 1, remote_address) <= 0) {
    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::passive_connection failed to send ack\n"));
  }

  const PriorityKey key = blob_to_key(blob, priority, cfg->send_receive_address(), false /* passive */);

  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connections_lock_);

  const PendConnMap::iterator pend = pending_connections_.find(key);

  if (pend != pending_connections_.end()) {

     //don't hold connections_lock_ while calling use_datalink
     //guard.release();

    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::passive_connection completing\n"));

    const DataLink_rch link = static_rchandle_cast<DataLink>(server_link_);

    //Insert key now to make sure when releasing guard to call use_datalink
    //if an accept_datalink obtains lock first it will see that it can proceed
    //with using the link and do its own use_datalink call.
    server_link_keys_.insert(key);

    //create a copy of the size of callback vector so that if use_datalink_i -> stop_accepting_or_connecting
    //finds that callbacks vector is empty and deletes pending connection & its callback vector for loop can
    //still exit the loop without checking the size of invalid memory
    //size_t num_callbacks = pend->second.size();

    //Create a copy of the vector of callbacks to process, making sure that each is
    //still present in the actual pending_connections_ before calling use_datalink
    Callbacks tmp(pend->second);
    for (size_t i = 0; i < tmp.size(); ++i) {
      const PendConnMap::iterator pos = pending_connections_.find(key);
      if (pos != pending_connections_.end()) {
        const Callbacks::iterator tmp_iter = find(pos->second.begin(),
                                                  pos->second.end(),
                                                  tmp.at(i));
        if (tmp_iter != pos->second.end()) {
          TransportClient_wrch pend_client = tmp.at(i).first;
          GUID_t remote_repo = tmp.at(i).second;
          guard.release();
          TransportClient_rch client = pend_client.lock();
          if (client)
            client->use_datalink(remote_repo, link);
          guard.acquire();
        }
      }
    }
  } else {
    // still hold guard(connections_lock_) at this point so
    // pending_server_link_keys_ is protected for insert

    VDBG((LM_DEBUG, "(%P|%t) UdpTransport::passive_connection pending\n"));
    // accept_datalink() will complete the connection.
    pending_server_link_keys_.insert(key);
  }
}

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
