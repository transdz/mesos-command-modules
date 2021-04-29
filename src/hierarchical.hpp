// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __MASTER_MY_ALLOCATOR_MESOS_HIERARCHICAL_HPP__
#define __MASTER_MY_ALLOCATOR_MESOS_HIERARCHICAL_HPP__

#include <memory>
#include <set>
#include <string>

#include <mesos/mesos.hpp>

#include <process/future.hpp>
#include <process/id.hpp>
#include <process/owned.hpp>

#include <stout/boundedhashmap.hpp>
#include <stout/duration.hpp>
#include <stout/hashmap.hpp>
#include <stout/hashset.hpp>
#include <stout/lambda.hpp>
#include <stout/option.hpp>

#include "common/protobuf_utils.hpp"

#include "master/allocator/mesos/allocator.hpp"
#include "metrics.hpp"

#include "master/allocator/mesos/sorter/drf/sorter.hpp"
#include "master/allocator/mesos/sorter/random/sorter.hpp"

#include "master/allocator/mesos/slavesorter/random/slavesorter.hpp"
#include "slavesorter/cpu_first/slavesorter.hpp"
#include "slavesorter/resources_weights/slavesorter.hpp"
#include "master/allocator/mesos/slavesorter/resources_weights/slavesorter.hpp"
#include "slavesorter/lexicographic/slavesorter.hpp"

#include "master/constants.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {

// We forward declare the hierarchical allocator process so that we
// can typedef an instantiation of it with DRF sorters.
template <
    typename RoleSorter,
    typename FrameworkSorter,
    typename SlaveSorter>
class MyAllocator;

typedef MyAllocator<DRFSorter, DRFSorter, RandomSlaveSorter> HierarchicalDRFRandomSortedSlavesAllocatorProcess;
typedef MesosAllocator<HierarchicalDRFRandomSortedSlavesAllocatorProcess> HierarchicalDRFRandomSortedSlavesAllocator;

typedef MyAllocator<DRFSorter, DRFSorter, MyCustomSlaveSorter> MyCustomSlaveSorterProcess;
typedef MesosAllocator<MyCustomSlaveSorterProcess> MyCustomAllocator;

typedef MyAllocator<DRFSorter, DRFSorter, MyResourceWeightedSlaveSorter> MyResourceWeightsSortedSlavesProcess;
typedef MesosAllocator<MyResourceWeightsSortedSlavesProcess> MyResourceWeightsSortedSlavesAllocator;

typedef MyAllocator<DRFSorter, DRFSorter, MyLexicographicSorter> MyLexicographicAllocatorProcess;
typedef MesosAllocator<MyLexicographicAllocatorProcess> MyLexicographicAllocator;

typedef MyAllocator<RandomSorter, RandomSorter, MyCustomSlaveSorter> HierarchicalRandomResourceSortedCPUFirstSlavesAllocatorProcess;
typedef MesosAllocator<HierarchicalRandomResourceSortedCPUFirstSlavesAllocatorProcess> HierarchicalRandomResourceSortedCPUFirstSlavesAllocator;

typedef MyAllocator<RandomSorter, RandomSorter, MyResourceWeightedSlaveSorter> HierarchicalRandomResourceSortedWeightsAllocatorProcess;
typedef MesosAllocator<HierarchicalRandomResourceSortedWeightsAllocatorProcess> HierarchicalRandomResourceSortedWeightsAllocator;

typedef MyAllocator<RandomSorter, RandomSorter, MyLexicographicSorter> HierarchicalRandomLexicographicSortedSlavesAllocatorProcess;
typedef MesosAllocator<HierarchicalRandomLexicographicSortedSlavesAllocatorProcess> HierarchicalRandomLexicographicSortedSlavesAllocator;

typedef MyAllocator<RandomSorter, RandomSorter, RandomSlaveSorter> HierarchicalRandomRandomSortedSlavesAllocatorProcess;
typedef MesosAllocator<HierarchicalRandomRandomSortedSlavesAllocatorProcess> HierarchicalRandomRandomSortedSlavesAllocator;

typedef HierarchicalDRFRandomSortedSlavesAllocatorProcess HierarchicalDRFAllocatorProcess;
typedef HierarchicalDRFRandomSortedSlavesAllocator HierarchicalDRFAllocator;

typedef HierarchicalRandomRandomSortedSlavesAllocatorProcess HierarchicalRandomAllocatorProcess;
typedef HierarchicalRandomRandomSortedSlavesAllocator HierarchicalRandomAllocator;

namespace internal {

// Forward declarations.
class OfferFilter;
class InverseOfferFilter;
class RoleTree;


struct Framework
{
  Framework(
      const FrameworkInfo& frameworkInfo,
      const std::set<std::string>& suppressedRoles,
      bool active,
      bool publishPerFrameworkMetrics);

  const FrameworkID frameworkId;

  std::set<std::string> roles;

  std::set<std::string> suppressedRoles;

  protobuf::framework::Capabilities capabilities;

  // Offer filters are tied to the role the filtered
  // resources were allocated to.
  hashmap<std::string, hashmap<SlaveID, hashset<std::shared_ptr<OfferFilter>>>>
    offerFilters;

  hashmap<SlaveID, hashset<std::shared_ptr<InverseOfferFilter>>>
    inverseOfferFilters;

  bool active;

  bool publishPerFrameworkMetrics;

  process::Owned<FrameworkMetrics> metrics;

  // TODO(bbannier): Consider documenting examples on how to use this setting.
  hashmap<std::string, std::vector<ResourceQuantities>> minAllocatableResources;
};


class Role
{
public:
  Role(const std::string& name, Role* parent);

  const ResourceQuantities& reservationScalarQuantities() const
  {
    return reservationScalarQuantities_;
  }

  const hashset<FrameworkID>& frameworks() const { return frameworks_; }

  const Quota& quota() const { return quota_; }

  double weight() const { return weight_; }

  bool isEmpty() const
  {
    return children_.empty() &&
           frameworks_.empty() &&
           reservationScalarQuantities_.empty() &&
           quota_ == DEFAULT_QUOTA &&
           weight_ == DEFAULT_WEIGHT;
  }

  std::vector<Role*> children() const { return children_.values(); }

  const std::string role; // E.g. "a/b/c"
  const std::string basename; // E.g. "c"

private:
  // We keep fields that are related to the tree structure as private
  // and only allow mutations through the RoleTree structure.
  friend class RoleTree;

  // Add a child to the role, the child must not already exist.
  void addChild(Role* child);

  // Remove a child from the role, the child must be present.
  void removeChild(Role* child);

  Role* parent;

  // Configured guaranteed resource quantities and resource limits for
  // this role. By default, a role has no guarantee and no limit.
  Quota quota_;

  // Configured weight for the role. This affects sorting precedence.
  // By default, weights == DEFAULT_WEIGHT == 1.0.
  double weight_;

  // IDs of the frameworks subscribed to the role, if any.
  hashset<FrameworkID> frameworks_;

  // Aggregated reserved scalar resource quantities on all agents tied to this
  // role, if any. This includes both its own reservations as well as
  // reservations of any of its subroles (i.e. it is hierarchical aware).
  // Note that non-scalar resources, such as ports, are excluded.
  ResourceQuantities reservationScalarQuantities_;

  hashmap<std::string, Role*> children_;
};


// A tree abstraction for organizing `class Role` hierarchically.
//
// We track a role when it has:
//
//   * a non-default weight, or
//   * a non-default quota, or
//   * frameworks subscribed to it, or
//   * reservations, or
//   * descendent roles meeting any of the above conditions.
//
// Any roles that do not meet these conditions are not tracked in the role tree.
class RoleTree
{
public:
  RoleTree(); // Only used in tests.

  RoleTree(Metrics* metrics);

  ~RoleTree();

  Option<const Role*> get(const std::string& role) const;

  // Return a hashmap of all known roles. Root is not included.
  const hashmap<std::string, Role>& roles() const { return roles_; }

  const Role* root() const { return root_; }

  // We keep track of reservations to enforce role quota limit
  // in the presence of unallocated reservations. See MESOS-4527.
  void trackReservations(const Resources& resources);
  void untrackReservations(const Resources& resources);

  void trackFramework(
    const FrameworkID& frameworkId, const std::string& role);
  void untrackFramework(
      const FrameworkID& frameworkId, const std::string& role);

  void updateQuota(const std::string& role, const Quota& quota);

  void updateWeight(const std::string& role, double weight);

  // Dump the role tree state in JSON format for debugging.
  std::string toJSON() const;

private:
  // Lookup or add the role struct associated with the role. Ancestor roles
  // along the tree path will be created if necessary.
  Role& operator[](const std::string& role);

  // Try to remove the role associated with the given role.
  // The role must exist. The role and its ancestors will be removed
  // if they become "empty". See "Role:isEmpty()".
  // Return true if the role instance associated with the role is removed.
  // This should be called whenever a role's state (that defines its emptiness)
  // gets updated, such as quota, weight, reservation and tracked frameworks.
  // Otherwise the "tracking only non-empty" tree invariant may break.
  bool tryRemove(const std::string& role);

  // Root node of the tree, its `basename` == `role` == "".
  Role* root_;

  // Allocator's metrics handle for publishing role related metrics.
  Option<Metrics*> metrics;

  // A map of role and `Role` pairs for quick lookup.
  hashmap<std::string, Role> roles_;
};


class Slave
{
public:
  Slave(
      const SlaveInfo& _info,
      const protobuf::slave::Capabilities& _capabilities,
      bool _activated,
      const Resources& _total,
      const Resources& _allocated)
    : info(_info),
      capabilities(_capabilities),
      activated(_activated),
      total(_total),
      allocated(_allocated),
      shared(_total.shared()),
      hasGpu_(_total.gpus().getOrElse(0) > 0)
  {
    updateAvailable();
  }

  const Resources& getTotal() const { return total; }

  const Resources& getAllocated() const { return allocated; }

  const Resources& getAvailable() const { return available; }

  bool hasGpu() const { return hasGpu_; }

  void updateTotal(const Resources& newTotal) {
    total = newTotal;
    shared = total.shared();
    hasGpu_ = total.gpus().getOrElse(0) > 0;

    updateAvailable();
  }

  void allocate(const Resources& toAllocate)
  {
    allocated += toAllocate;

    updateAvailable();
  }

  void unallocate(const Resources& toUnallocate)
  {
    allocated -= toUnallocate;

    updateAvailable();
  }

  // The `SlaveInfo` that was passed to the allocator when the slave was added
  // or updated. Currently only two fields are used: `hostname` for host
  // whitelisting and in log messages, and `domain` for region-aware
  // scheduling.
  SlaveInfo info;

  protobuf::slave::Capabilities capabilities;

  bool activated; // Whether to offer resources.

  // Represents a scheduled unavailability due to maintenance for a specific
  // slave, and the responses from frameworks as to whether they will be able
  // to gracefully handle this unavailability.
  //
  // NOTE: We currently implement maintenance in the allocator to be able to
  // leverage state and features such as the FrameworkSorter and OfferFilter.
  struct Maintenance
  {
    Maintenance(const Unavailability& _unavailability)
      : unavailability(_unavailability) {}

    // The start time and optional duration of the event.
    Unavailability unavailability;

    // A mapping of frameworks to the inverse offer status associated with
    // this unavailability.
    //
    // NOTE: We currently lose this information during a master fail over
    // since it is not persisted or replicated. This is ok as the new master's
    // allocator will send out new inverse offers and re-collect the
    // information. This is similar to all the outstanding offers from an old
    // master being invalidated, and new offers being sent out.
    hashmap<FrameworkID, mesos::allocator::InverseOfferStatus> statuses;

    // Represents the "unit of accounting" for maintenance. When a
    // `FrameworkID` is present in the hashset it means an inverse offer has
    // been sent out. When it is not present it means no offer is currently
    // outstanding.
    hashset<FrameworkID> offersOutstanding;
  };

  // When the `maintenance` is set the slave is scheduled to be unavailable at
  // a given point in time, for an optional duration. This information is used
  // to send out `InverseOffers`.
  Option<Maintenance> maintenance;

private:
  void updateAvailable() {
    // In order to subtract from the total,
    // we strip the allocation information.
    Resources allocated_ = allocated;
    allocated_.unallocate();

    // Calling `nonShared()` currently copies the underlying resources
    // and is therefore rather expensive. We avoid it in the common
    // case that there are no shared resources.
    //
    // TODO(mzhu): Ideally there would be a single logical path here.
    // One solution is to have `Resources` be copy-on-write such that
    // `nonShared()` performs no copying and instead points to a
    // subset of the original `Resource` objects.
    if (shared.empty()) {
      available = total - allocated_;
    } else {
      // Since shared resources are offerable even when they are in use, we
      // always include them as part of available resources.
      available = (total.nonShared() - allocated_.nonShared()) + shared;
    }
  }

  // Total amount of regular *and* oversubscribed resources.
  Resources total;

  // Regular *and* oversubscribed resources that are allocated.
  //
  // NOTE: We maintain multiple copies of each shared resource allocated
  // to a slave, where the number of copies represents the number of times
  // this shared resource has been allocated to (and has not been recovered
  // from) a specific framework.
  //
  // NOTE: We keep track of the slave's allocated resources despite
  // having that information in sorters. This is because the
  // information in sorters is not accurate if some framework
  // hasn't reregistered. See MESOS-2919 for details.
  Resources allocated;

  // We track the total and allocated resources on the slave to
  // avoid calculating it in place every time.
  //
  // Note that `available` always contains all the shared resources on the
  // agent regardless whether they have ever been allocated or not.
  // NOTE, however, we currently only offer a shared resource only if it has
  // not been offered in an allocation cycle to a framework. We do this mainly
  // to preserve the normal offer behavior. This may change in the future
  // depending on use cases.
  //
  // Note that it's possible for the slave to be over-allocated!
  // In this case, allocated > total.
  Resources available;

  // We keep a copy of the shared resources to avoid unnecessary copying.
  Resources shared;

  // We cache whether the agent has gpus as an optimization.
  bool hasGpu_;
};


// Implements the basic allocator algorithm - first pick a role by
// some criteria, then pick one of their frameworks to allocate to.
class MyAllocator : public MesosAllocatorProcess
{
public:
  MyAllocator(
      const std::function<Sorter*()>& roleSorterFactory,
      const std::function<Sorter*()>& _frameworkSorterFactory,
      const std::function<SlaveSorter*()>& slaveSorterFactory
      )
    : initialized(false),
      paused(true),
      metrics(*this),
      completedFrameworkMetrics(0),
      roleTree(&metrics),
      roleSorter(roleSorterFactory()),
      frameworkSorterFactory(_frameworkSorterFactory),
      slaveSorter(slaveSorterFactory()) {}

  ~MyAllocator() override {}

  process::PID<MyAllocator> self() const
  {
    return process::PID<Self>(this);
  }

  void initialize(
      const mesos::allocator::Options& options,
      const lambda::function<
          void(const FrameworkID&,
               const hashmap<std::string, hashmap<SlaveID, Resources>>&)>&
        offerCallback,
      const lambda::function<
          void(const FrameworkID&,
               const hashmap<SlaveID, UnavailableResources>&)>&
        inverseOfferCallback) override;

  void recover(
      const int _expectedAgentCount,
      const hashmap<std::string, Quota>& quotas) override;

  void addFramework(
      const FrameworkID& frameworkId,
      const FrameworkInfo& frameworkInfo,
      const hashmap<SlaveID, Resources>& used,
      bool active,
      const std::set<std::string>& suppressedRoles) override;

  void removeFramework(
      const FrameworkID& frameworkId) override;

  void activateFramework(
      const FrameworkID& frameworkId) override;

  void deactivateFramework(
      const FrameworkID& frameworkId) override;

  void updateFramework(
      const FrameworkID& frameworkId,
      const FrameworkInfo& frameworkInfo,
      const std::set<std::string>& suppressedRoles) override;

  void addSlave(
      const SlaveID& slaveId,
      const SlaveInfo& slaveInfo,
      const std::vector<SlaveInfo::Capability>& capabilities,
      const Option<Unavailability>& unavailability,
      const Resources& total,
      const hashmap<FrameworkID, Resources>& used) override;

  void removeSlave(
      const SlaveID& slaveId) override;

  void updateSlave(
      const SlaveID& slave,
      const SlaveInfo& slaveInfo,
      const Option<Resources>& total = None(),
      const Option<std::vector<SlaveInfo::Capability>>& capabilities = None())
    override;

  void addResourceProvider(
      const SlaveID& slave,
      const Resources& total,
      const hashmap<FrameworkID, Resources>& used) override;

  void deactivateSlave(
      const SlaveID& slaveId) override;

  void activateSlave(
      const SlaveID& slaveId) override;

  void updateWhitelist(
      const Option<hashset<std::string>>& whitelist) override;

  void requestResources(
      const FrameworkID& frameworkId,
      const std::vector<Request>& requests) override;

  void updateAllocation(
      const FrameworkID& frameworkId,
      const SlaveID& slaveId,
      const Resources& offeredResources,
      const std::vector<ResourceConversion>& conversions) override;

  process::Future<Nothing> updateAvailable(
      const SlaveID& slaveId,
      const std::vector<Offer::Operation>& operations) override;

  void updateUnavailability(
      const SlaveID& slaveId,
      const Option<Unavailability>& unavailability) override;

  void updateInverseOffer(
      const SlaveID& slaveId,
      const FrameworkID& frameworkId,
      const Option<UnavailableResources>& unavailableResources,
      const Option<mesos::allocator::InverseOfferStatus>& status,
      const Option<Filters>& filters) override;

  process::Future<
      hashmap<SlaveID,
      hashmap<FrameworkID, mesos::allocator::InverseOfferStatus>>>
    getInverseOfferStatuses() override;

  void recoverResources(
      const FrameworkID& frameworkId,
      const SlaveID& slaveId,
      const Resources& resources,
      const Option<Filters>& filters) override;

  void suppressOffers(
      const FrameworkID& frameworkId,
      const std::set<std::string>& roles) override;

  void reviveOffers(
      const FrameworkID& frameworkId,
      const std::set<std::string>& roles) override;

  void updateQuota(
      const std::string& role,
      const Quota& quota) override;

  void updateWeights(
      const std::vector<WeightInfo>& weightInfos) override;

  void pause() override;

  void resume() override;

protected:
  // Useful typedefs for dispatch/delay/defer to self()/this.
  typedef MyAllocator Self;
  typedef MyAllocator This;

  // Allocate any allocatable resources from all known agents.
  process::Future<Nothing> allocate();

  // Allocate resources from the specified agent.
  process::Future<Nothing> allocate(const SlaveID& slaveId);

  // Allocate resources from the specified agents. The allocation
  // is deferred and batched with other allocation requests.
  process::Future<Nothing> allocate(const hashset<SlaveID>& slaveIds);

  // Method that performs allocation work.
  Nothing _allocate();

  // Helper for `_allocate()` that allocates resources for offers.
  void __allocate();

  // Helper for `_allocate()` that deallocates resources for inverse offers.
  void deallocate();

  // Remove an offer filter for the specified role of the framework.
  void expire(
      const FrameworkID& frameworkId,
      const std::string& role,
      const SlaveID& slaveId,
      const std::weak_ptr<OfferFilter>& offerFilter);

  void _expire(
      const FrameworkID& frameworkId,
      const std::string& role,
      const SlaveID& slaveId,
      const std::weak_ptr<OfferFilter>& offerFilter);

  // Remove an inverse offer filter for the specified framework.
  void expire(
      const FrameworkID& frameworkId,
      const SlaveID& slaveId,
      const std::weak_ptr<InverseOfferFilter>& inverseOfferFilter);

  // Checks whether the slave is whitelisted.
  bool isWhitelisted(const SlaveID& slaveId) const;

  // Returns true if there is a resource offer filter for the
  // specified role of this framework on this slave.
  bool isFiltered(
      const Framework& framework,
      const std::string& role,
      const Slave& slave,
      const Resources& resources) const;

  // Returns true if there is an inverse offer filter for this framework
  // on this slave.
  bool isFiltered(
      const Framework& framework,
      const Slave& slave) const;

  bool allocatable(
      const Resources& resources,
      const std::string& role,
      const Framework& framework) const;

  bool initialized;
  bool paused;

  mesos::allocator::Options options;

  // Recovery data.
  Option<int> expectedAgentCount;
  float agentRecoveryFactor;
  Duration recoveryTimeout;

  lambda::function<
      void(const FrameworkID&,
           const hashmap<std::string, hashmap<SlaveID, Resources>>&)>
    offerCallback;

  lambda::function<
      void(const FrameworkID&,
           const hashmap<SlaveID, UnavailableResources>&)>
    inverseOfferCallback;

  friend Metrics;
  Metrics metrics;

  double _event_queue_dispatches()
  {
    return static_cast<double>(eventCount<process::DispatchEvent>());
  }

  double _resources_total(
      const std::string& resource);

  double _resources_offered_or_allocated(
      const std::string& resource);

  double _quota_allocated(
      const std::string& role,
      const std::string& resource);

  double _offer_filters_active(
      const std::string& role);

  hashmap<FrameworkID, Framework> frameworks;

  BoundedHashMap<FrameworkID, process::Owned<FrameworkMetrics>>
    completedFrameworkMetrics;

  hashmap<SlaveID, Slave> slaves;

  RoleTree roleTree;

  // A set of agents that are kept as allocation candidates. Events
  // may add or remove candidates to the set. When an allocation is
  // processed, the set of candidates is cleared.
  hashset<SlaveID> allocationCandidates;

  // Future for the dispatched allocation that becomes
  // ready after the allocation run is complete.
  Option<process::Future<Nothing>> allocation;

  // Slaves to send offers for.
  Option<hashset<std::string>> whitelist;

  // There are two stages of allocation:
  //
  //   Stage 1: Allocate to satisfy quota guarantees.
  //
  //   Stage 2: Allocate above quota guarantees up to quota limits.
  //            Note that we need to hold back enough "headroom"
  //            to ensure that any unsatisfied quota can be
  //            satisfied later.
  //
  // Each stage comprises two levels of sorting, hence "hierarchical".
  // Level 1 sorts across roles:
  //   Currently, only the allocated portion of the reserved resources are
  //   accounted for fairness calculation.
  //
  // TODO(mpark): Reserved resources should be accounted for fairness
  // calculation whether they are allocated or not, since they model a long or
  // forever running task. That is, the effect of reserving resources is
  // equivalent to launching a task in that the resources that make up the
  // reservation are not available to other roles as non-revocable.
  //
  // Level 2 sorts across frameworks within a particular role:
  //   Reserved resources at this level are, and should be accounted for
  //   fairness calculation only if they are allocated. This is because
  //   reserved resources are fairly shared across the frameworks in the role.
  //
  // The allocator relies on `Sorter`s to employ a particular sorting
  // algorithm. Each level has its own sorter and hence may have different
  // fairness calculations.
  //
  // NOTE: The hierarchical allocator considers revocable resources as
  // regular resources when doing fairness calculations.
  //
  // TODO(vinod): Consider using a different fairness algorithm for
  // revocable resources.

  // A sorter for active roles. This sorter determines the order in which
  // roles are allocated resources during Level 1 of the second stage.
  // The total cluster resources are used as the resource pool.
  process::Owned<Sorter> roleSorter;

  // A collection of sorters, one per active role. Each sorter determines
  // the order in which frameworks that belong to the same role are allocated
  // resources inside the role's share. These sorters are used during Level 2
  // for both the first and the second stages. Since frameworks are sharing
  // the resources allocated to a role, the role's allocation is used as
  // the resource pool for each role specific framework sorter.
  hashmap<std::string, process::Owned<Sorter>> frameworkSorters;

  // Factory function for framework sorters.
  const std::function<Sorter*()> frameworkSorterFactory;

  //TODO(ojabnoune) DOC
  process::Owned<SlaveSorter> slaveSorter;

private:
  bool isFrameworkTrackedUnderRole(
      const FrameworkID& frameworkId,
      const std::string& role) const;

  Option<Slave*> getSlave(const SlaveID& slaveId) const;
  Option<Framework*> getFramework(const FrameworkID& frameworkId) const;

  Option<Sorter*> getFrameworkSorter(const std::string& role) const;

  const Quota& getQuota(const std::string& role) const;

  void trackFrameworkUnderRole(
      const FrameworkID& frameworkId,
      const std::string& role);

  void untrackFrameworkUnderRole(
      const FrameworkID& frameworkId,
      const std::string& role);

  void suppressRoles(Framework& framework, const std::set<std::string>& roles);
  void reviveRoles(Framework& framework, const std::set<std::string>& roles);

  // Helper to update the agent's total resources maintained in the allocator
  // and the role and quota sorters (whose total resources match the agent's
  // total resources). Returns true iff the stored agent total was changed.
  bool updateSlaveTotal(const SlaveID& slaveId, const Resources& total);

  // Helper that returns true if the given agent is located in a
  // different region than the master. This can only be the case if
  // the agent and the master are both configured with a fault domain.
  bool isRemoteSlave(const Slave& slave) const;

  // Helper function that checks if a framework is capable of
  // receiving resources on the agent based on the framework capability.
  //
  // TODO(mzhu): Make this a `Framework` member function once we pull
  // `struct Framework` out from being nested.
  bool isCapableOfReceivingAgent(
      const protobuf::framework::Capabilities& frameworkCapabilities,
      const Slave& slave) const;

  // Helper function that removes any resources that the framework is not
  // capable of receiving based on the given framework capability.
  //
  // TODO(mzhu): Make this a `Framework` member function once we pull
  // `struct Framework` out from being nested.
  Resources stripIncapableResources(
      const Resources& resources,
      const protobuf::framework::Capabilities& frameworkCapabilities) const;

  // Helper to track allocated resources on an agent.
  void trackAllocatedResources(
      const SlaveID& slaveId,
      const FrameworkID& frameworkId,
      const Resources& allocated);

  // Helper to untrack resources that are no longer allocated on an agent.
  void untrackAllocatedResources(
      const SlaveID& slaveId,
      const FrameworkID& frameworkId,
      const Resources& allocated);

  // Helper that removes all existing offer filters for the given slave
  // id.
  void removeFilters(const SlaveID& slaveId);
};


} // namespace internal {


// We map the templatized version of the `HierarchicalAllocatorProcess` to one
// that relies on sorter factories in the internal namespace. This allows us
// to keep the implementation of the allocator in the implementation file.
template <
    typename RoleSorter,
    typename FrameworkSorter,
    typename SlaveSorter>
class MyAllocator
  : public internal::MyAllocator
{
public:
  MyAllocator()
    : ProcessBase(process::ID::generate("hierarchical-allocator")),
      internal::MyAllocator(
          [this]() -> Sorter* {
            return new RoleSorter(this->self(), "allocator/mesos/roles/");
          },
          []() -> Sorter* { return new FrameworkSorter(); },
          []() -> SlaveSorter* { return new SlaveSorter(); }) {}
};

} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {

#endif // __MASTER_ALLOCATOR_MESOS_HIERARCHICAL_HPP__
