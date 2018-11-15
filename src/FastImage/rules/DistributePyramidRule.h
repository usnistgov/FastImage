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


/// @file DistributePyramidRule.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/25/17
/// @brief Pyramid release rule

#ifndef FAST_IMAGE_DISTRIBUTEPYRAMIDRULE_H
#define FAST_IMAGE_DISTRIBUTEPYRAMIDRULE_H

#include <htgs/api/IRule.hpp>
#include "FastImage/data/ViewRequestData.h"

namespace fi {
/// \namespace fi FastImage namespace

/// \brief
/**
  * @class DistributePyramidRule DistributePyramidRule.h <FastImage/rules/DistributePyramidRule.h>
  *
  * @brief Distribute Pyramid Rule used by the execution pipeline
  * @tparam UserType Pixel Type asked by the end use
  **/
template<typename UserType>
class DistributePyramidRule
    : public htgs::IRule<fi::ViewRequestData<UserType>,
                         fi::ViewRequestData<UserType>> {
 public:
  /// \brief Distributed Pyramid Rule constructor
  DistributePyramidRule() : htgs::IRule<fi::ViewRequestData<UserType>,
                                        fi::ViewRequestData<UserType>>() {}

  /// \brief Get rule name
  /// \return Rule Name
  std::string getName() { return "DistributePyramidRule"; }

  /// \brief Apply the rule to the data
  /// \param data to apply the rule to
  /// \param pipelineId to apply the rule to
  void applyRule(std::shared_ptr<fi::ViewRequestData<UserType>> data,
                 size_t pipelineId) {
    if (data->getLevel() == pipelineId) { this->addResult(data); }
  }
};
}

#endif //FAST_IMAGE_DISTRIBUTEPYRAMIDRULE_H
