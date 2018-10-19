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

/// @file ViewAnalyse.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  4/6/18
/// @brief Main interface to construct a tile loader


#ifndef REGIONLABELING_VIEWANALYSE_H
#define REGIONLABELING_VIEWANALYSE_H

#include <unordered_map>
#include <htgs/api/IData.hpp>

#include "Blob.h"

namespace fc {
/// \namespace fc FeatureCollection namespace


/**
  * @class ViewAnalyse ViewAnalyse.h <FastImage/FeatureCollection/Data//ViewAnalyse.h>
  *
  * @brief Describe a collection of blob in a view. Derive from IData.
  *
  * @details View analyse, designed and use to keep track of the different
  * blobs in a specific view. The blobs in the border could be merged to another
  * blob in an adjacent view. They are placed in the _toMerge map, the key is
  * the blob address and the values are all the possible coordinates the blob can
  * merge with.
  **/
class ViewAnalyse : public htgs::IData {
 public:
  /// \brief Getter to the merge map
  /// \return Merge map
  const std::unordered_map<Blob *, std::list<Coordinate>> &getToMerge() const {
    return _toMerge;
  }
  /// \brief Getter to the blobs list
  /// \return The blobs list
  const std::list<Blob *> &getBlobs() const { return _blobs; }

  /// \brief Add an entry to the to merge structure
  /// \param b Blob to add
  /// \param c Coordinate links to this  blob
  void addToMerge(Blob *b, Coordinate c) {_toMerge[b].push_back(c);}

  /// \brief Insert a blob to the list of blobs
  /// \param b blob to add
  void insertBlob(Blob *b) { _blobs.push_back(b); }

 private:
  std::unordered_map<fc::Blob *, std::list<Coordinate>>
      _toMerge;   ///< Map of blob which will need to be merged to a list of
                  ///< coordinates

  std::list<Blob *>
      _blobs;   ///< List of blobs created
};
}
#endif //REGIONLABELING_VIEWANALYSE_H
