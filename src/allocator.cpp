#include <mesos/allocator/allocator.hpp>
#include <mesos/module/allocator.hpp>
#include <stout/try.hpp>

#include "allocator.hpp"

using mesos::internal::master::allocator::HierarchicalDRFAllocator;
using mesos::internal::master::allocator::HierarchicalRandomAllocator;

using mesos::internal::master::allocator::HierarchicalDRFRandomSortedSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalDRFResourceSortedSlavesCPUFirstAllocator;
using mesos::internal::master::allocator::HierarchicalDRFResourceWeightsSortedSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalDRFLexicographicSortedSlavesAllocator;

using mesos::internal::master::allocator::HierarchicalRandomResourceSortedCPUFirstSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalRandomResourceSortedWeightsAllocator;
using mesos::internal::master::allocator::HierarchicalRandomLexicographicSortedSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalRandomRandomSortedSlavesAllocator;

using namespace mesos;
using mesos::allocator::Allocator;
using mesos::internal::master::allocator::HierarchicalDRFAllocator;

static Allocator* createExternalAllocator(const Parameters& parameters)
{
  Try<Allocator*> allocator = ExternalAllocator::create();
  if (allocator.isError()) {
    return nullptr;
  }

  return allocator.get();
}

// Declares an ExternalAllocator module named 'ExternalAllocatorModule'.
mesos::modules::Module<Allocator> ExternalAllocatorModule(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Mesos Contributor",
    "engineer@example.com",
    "External Allocator module.",
    nullptr,
    createExternalAllocator);