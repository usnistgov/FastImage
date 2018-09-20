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


/// @file FastImageException.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  11/5/17
/// @brief Special Exception for Fast Image


#ifndef FAST_IMAGE_FASTIMAGEXCEPTION_H
#define FAST_IMAGE_FASTIMAGEXCEPTION_H

#include <utility>
#include <exception>
#include <string>

/// \namespace fi FastImage namespace
namespace fi {

/**
  * @class FastImageException FastImageException.h <FastImage/exception/FastImageException.h>
  *
  * @brief Exception specialized for Fast Image
  **/

class FastImageException : public std::exception {
 public:
  /// \brief FastImageException constructor
  /// \param message Exception message
  explicit FastImageException(std::string message = "") noexcept
      : _message(std::move(message)) {}

  /// \brief FastImageException copy constructor
  /// \param exception FastImageException to copy
  FastImageException(FastImageException &&exception) noexcept {
    this->_message = exception.get_message();
  }

  /// \brief FastImageException destructor
  ~FastImageException() noexcept override = default;

  /// \brief Message getter
  /// \return message
  const std::string &get_message() const { return _message; }

  /// \brief Returns a C-style character string describing the general cause of the
  /// current error
  /// \return C-style character string describing the general cause of the
  /// current error
  const char *what() const noexcept override {
    return _message.c_str();
  }

 private:
  std::string _message;            ///< Error message
};

}
#endif //FAST_IMAGE_FASTIMAGEXCEPTION_H
