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

//
// Created by anb22 on 8/1/17.
//

/// @file ViewAllocator.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  8/1/17
/// @brief View Allocator used by the HTGS memory manager

#ifndef FASTIMAGE_VIEWALLOCATOR_H
#define FASTIMAGE_VIEWALLOCATOR_H

#include <htgs/api/IMemoryAllocator.hpp>
#include "../api/View.h"

namespace fi {
/// \namespace fi FastImage namespace

/**
  * @class ViewAllocator ViewAllocator.h <FastImage/memory/ViewAllocator.h>
  * @brief View Allocator allocates a FastImage view that is used by the HTGS
  * memory manager
  * @tparam UserType Pixel Type asked by the end user
  **/
template<typename UserType>
class ViewAllocator : public htgs::IMemoryAllocator<fi::View<UserType>> {
 public:
  /// \brief View allocator constructor
  /// \param viewHeight View Height in pixel
  /// \param viewWidth View width in pixel
  ViewAllocator(const uint32_t &viewHeight, const uint32_t &viewWidth) :
      htgs::IMemoryAllocator<fi::View<UserType>>(0),
      _viewHeight(viewHeight), _viewWidth(viewWidth) {}

  /// \brief Allocate the memory in a view
  /// \param size Bot used but needed by HTGS
  /// \return A new ViewData allocated
  View<UserType> *memAlloc(size_t size) override {
    return new View<UserType>(_viewHeight, _viewWidth);
  }

  /// \brief Allocate the memory in a view
  /// \return A new ViewData allocated
  View<UserType> *memAlloc() override {
    return new View<UserType>(_viewHeight, _viewWidth);
  }

  /// \brief Free a viewData
  /// \param memory viewData to free
  void memFree(View<UserType> *&memory) override {
    delete memory;
  }

 private:
  uint32_t
      _viewHeight,   ///< View height
      _viewWidth;    ///< View Width
};
}

#endif //FASTIMAGE_VIEWALLOCATOR_H
