// NIST-developed software is provided by NIST as a public service. 
// You may use, copy and distribute copies of the  software in any  medium, 
// provided that you keep intact this entire notice. You may improve, 
// modify and create derivative works of the software or any portion of the 
// software, and you may copy and distribute such modifications or works. 
// Modified works should carry a notice stating that you changed the software 
// and should note the date and nature of any such change. Please explicitly 
// acknowledge the National Institute of Standards and Technology as the 
// source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY
// OF ANY KIND, EXPRESS, IMPLIED, IN FACT  OR ARISING BY OPERATION OF LAW, 
// INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST 
// NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION  OF THE SOFTWARE WILL 
// BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST 
// DOES NOT WARRANT  OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE 
// SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE 
// CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using 
// and distributing the software and you assume  all risks associated with 
// its use, including but not limited to the risks and costs of program 
// errors, compliance  with applicable laws, damage to or loss of data, 
// programs or equipment, and the unavailability or interruption of operation. 
// This software is not intended to be used in any situation where a failure 
// could cause risk of injury or damage to property. The software developed 
// by NIST employees is not subject to copyright protection within 
// the United States.

/// @file VariableMemoryManager.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  11/24/17
/// @brief Variable Memory manager used to manage the views depending on the pyramid level

#ifndef FASTIMAGE_VARIABLEMEMORYMANAGER_H
#define FASTIMAGE_VARIABLEMEMORYMANAGER_H

#include <htgs/core/memory/MemoryManager.hpp>
#include <utility>

namespace fi {
/// \namespace fi FastImage namespace

/**
  * @class VariableMemoryManager VariableMemoryManager.h <FastImage/memory/VariableMemoryManager.h>
  *
  * @brief Variable Memory manager used to manage the memory for each
  * pyramid level
  *
  * @tparam T Representing a View<UserType>
  **/
template<class T>
class VariableMemoryManager : public htgs::MemoryManager<T> {
 public:
  /// \brief Memory manager constructor
  /// \param name Memory manager name
  /// \param memoryPoolSizes Pool size
  /// \param memoryAllocators Memory allocator
  /// \param type Memory manager type
  VariableMemoryManager(const std::string &name,
                        std::vector<size_t> memoryPoolSizes,
                        std::vector<std::shared_ptr<htgs::IMemoryAllocator<T>>>
                        memoryAllocators,
                        htgs::MMType type)
      : htgs::MemoryManager<T>(name, 0, nullptr, type),
        _memoryPoolSizes(std::move(memoryPoolSizes)),
        _memoryAllocators(memoryAllocators) {
  }

  /// \brief Get memory pool size
  /// \return Memory pool size
  size_t getMemoryPoolSize() override {
    return this->_memoryPoolSizes[this->getPipelineId()];
  }

  /// \brief Get memory allocator
  /// \return Memory allocator
  std::shared_ptr<htgs::IMemoryAllocator<T>> getAllocator() override {
    return _memoryAllocators[this->getPipelineId()];
  }

  /// \brief Copy Function used by HTGS
  /// \return Copied Variable Memory Manager
  VariableMemoryManager *copy() override {
    return new VariableMemoryManager(this->getMemoryManagerName(),
                                     _memoryPoolSizes,
                                     _memoryAllocators,
                                     this->getType());
  }

  /// \brief Get Name
  /// \return Name
  std::string getName() override {
    return "Variable" + htgs::MemoryManager<T>::getName();
  }

 private:
  std::vector<size_t>
      _memoryPoolSizes;   ///< Memory pool size

  std::vector<std::shared_ptr<htgs::IMemoryAllocator<T>>>
      _memoryAllocators;  ///< Memory allocators
};
}

#endif //FASTIMAGE_VARIABLEMEMORYMANAGER_H
