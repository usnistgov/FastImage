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

/// @file ReleaseCountRule.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/25/17
/// @brief Release rule for the views.

#ifndef FASTIMAGE_RELEASECOUNTRULE_H
#define FASTIMAGE_RELEASECOUNTRULE_H

#include <cstdint>
#include <htgs/api/IMemoryReleaseRule.hpp>

namespace fi {
/// \namespace fi FastImage namespace

/**
  * @class ReleaseCountRule ReleaseCountRule.h <FastImage/rules/ReleaseCountRule.h>
  *
  * @brief Memory management release rule based on a count that is decremented
  * after each use, and released when the count reaches 0.
  **/
class ReleaseCountRule : public htgs::IMemoryReleaseRule {
 public:
  /// \brief Release Rule for views
  /// \param releaseCount Number of time the view need to be ask for release to
  /// actually be released
  explicit ReleaseCountRule(uint32_t releaseCount)
      : _releaseCount(releaseCount) {}

  /// \brief Indicate the memory has been used
  void memoryUsed() override { --_releaseCount; }

  /// \brief Test if the memory can be released
  /// \return True if the memory can be released, False else
  bool canReleaseMemory() override {
    return _releaseCount == 0;
  }
 private:
  uint32_t
      _releaseCount = 1;  ///< Number of time the view need to be ask for
  ///< release to actually  be released
};
}
#endif //FASTIMAGE_RELEASECOUNTRULE_H
