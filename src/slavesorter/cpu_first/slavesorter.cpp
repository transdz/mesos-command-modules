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
#include "slavesorter/cpu_first/slavesorter.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {


MyCustomSlaveSorter::MyCustomSlaveSorter() {}

MyCustomSlaveSorter::~MyCustomSlaveSorter() {}

bool MyCustomSlaveSorter::_compare(SlaveID& l, SlaveID& r)
{
 
 CHECK(total_.resources.contains(l));
 CHECK(total_.resources.contains(r));
  const Resources &lres = total_.resources[l];
  const Resources &rres = total_.resources[r];
  if (lres.cpus().get() < rres.cpus().get()){
    return false;
  }
  else if (lres.cpus().get() > rres.cpus().get()) {
    return true;
  }

  if (lres.mem().get() < rres.mem().get()){
    return false;
  }else if (lres.mem().get() > rres.mem().get()) {
    return true;
  }

  return  (lres.disk().get() > rres.disk().get());
}

void MyCustomSlaveSorter::sort(
  std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end)
{ 
  LOG(INFO) << "Before sort with CPU";
   for (std::vector<SlaveID>::iterator it = begin; it != end; ++it)
    LOG(INFO) << *it;

  std::sort(
    begin, end, [this](SlaveID l, SlaveID r) { return _compare(l, r); });
  LOG(INFO) << "After sort with CPU";

   for (std::vector<SlaveID>::iterator it = begin; it != end; ++it)
    {
      std::ostringstream stream;
      stream << *it ;
      std::string str =  stream.str();
      std::size_t found = str.find_last_of("-");
      std::string texte = str.substr(found+1);
      const Resources &res = total_.resources[*it];
      LOG(INFO) << "Server" << texte << " with "<< res.cpus().get() ;
      
    }


}

void MyCustomSlaveSorter::add(
  const SlaveID& slaveId,
  const SlaveInfo& slaveInfo,
  const Resources& resources)
{
  LOG(INFO) << " added a new slave" << slaveId;
  // TODO(jabnouneo): refine
  // totalResources[slaveId] += resources.createStrippedScalarQuantity();
  if (!resources.empty()) {
    // Add shared resources to the total quantities when the same
    // resources don't already exist in the total.
    const Resources newShared =
      resources.shared().filter([this, slaveId](const Resource& resource) {
        return !total_.resources[slaveId].contains(resource);
      });

    total_.resources[slaveId] += resources;

    const Resources scalarQuantities =
      (resources.nonShared() + newShared).createStrippedScalarQuantity();

    total_.scalarQuantities += scalarQuantities;
  }
}

void MyCustomSlaveSorter::remove(
  const SlaveID& slaveId, const Resources& resources)
{
  if (!resources.empty()) {
    CHECK(total_.resources.contains(slaveId));
    CHECK(total_.resources[slaveId].contains(resources))
      << total_.resources[slaveId] << " does not contain " << resources;

    total_.resources[slaveId] -= resources;

    // Remove shared resources from the total quantities when there
    // are no instances of same resources left in the total.
    const Resources absentShared =
      resources.shared().filter([this, slaveId](const Resource& resource) {
        return !total_.resources[slaveId].contains(resource);
      });

    const Resources scalarQuantities =
      (resources.nonShared() + absentShared).createStrippedScalarQuantity();

    CHECK(total_.scalarQuantities.contains(scalarQuantities));
    total_.scalarQuantities -= scalarQuantities;

    if (total_.resources[slaveId].empty()) {
      total_.resources.erase(slaveId);
    }
  }
}

void MyCustomSlaveSorter::allocated(
  const SlaveID& slaveId, const Resources& toAdd)
{
  LOG(INFO) << "allocated a new resource from "<< slaveId;
  // Add shared resources to the allocated quantities when the same
  // resources don't already exist in the allocation.
  const Resources sharedToAdd =
    toAdd.shared().filter([this, slaveId](const Resource& resource) {
      return !total_.resources[slaveId].contains(resource);
    });

  const Resources quantitiesToAdd =
    (toAdd.nonShared() + sharedToAdd).createStrippedScalarQuantity();
  total_.resources[slaveId] += quantitiesToAdd;
  allocatedResources[slaveId] += toAdd;
  total_.scalarQuantities += quantitiesToAdd;
}

// Specify that resources have been unallocated on the given slave.
void MyCustomSlaveSorter::unallocated(
  const SlaveID& slaveId, const Resources& toRemove)
{
  // TODO(jabnouneo): refine and account for shared resources
  CHECK(allocatedResources.contains(slaveId));
  CHECK(allocatedResources.at(slaveId).contains(toRemove))
    << "Resources " << allocatedResources.at(slaveId) << " at agent " << slaveId
    << " does not contain " << toRemove;

  allocatedResources[slaveId] -= toRemove;


  if (allocatedResources[slaveId].empty()) {
    allocatedResources.erase(slaveId);
  }
}

} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {
