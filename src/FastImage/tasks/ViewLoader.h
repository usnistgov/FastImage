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

/// @file ViewLoader.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/31/17
/// @brief View Loader task.

#ifndef FASTIMAGE_VIEWLOADER_H
#define FASTIMAGE_VIEWLOADER_H

#include <utility>

#include <htgs/api/ITask.hpp>

#include "FastImage/rules/ReleaseCountRule.h"
#include "FastImage/data/ViewRequestData.h"
#include "FastImage/data/TileRequestData.h"

namespace fi {
/// \namespace fi FastImage namespace

/**
  * @class ViewLoader ViewLoader.h <FastImage/tasks/ViewLoader.h>
  *
  * @brief View loader task. Take a ViewRequestData and produce a
  * TileRequestData.
  *
  * @details First task of the Fast Image graph. The goal of the ViewLoader is
  * to use the fi::ViewRequestData<UserType> to generate n
  * fi::TileRequestData<UserType> for the ATileLoader used.
  * The fi::ViewRequestData<UserType> describes which view is requested by
  * the end user. The fi::TileRequestData<UserType> describes which part of
  * the image has to be loaded and where it has to be copied in the view.
  * It will use the MemoryManager to get an empty view, or wait until a view has
  * been released. The number of views available to the memory manager can
  * be specified from
  * fi::FastImage->getFastImageOptions()->setNumberOfViewParallel().
  *
  * @tparam UserType Pixel Type asked by the end user
  **/

template<typename UserType>
class ViewLoader : public htgs::ITask<fi::ViewRequestData<UserType>,
                                      fi::TileRequestData<UserType>> {
 public:
  /// \brief Create a view loader and assign the count of release for each
  /// pyramid levels
  /// \param nbReleasePyramid Number of times a view need to be release for each
  /// pyramid level.
  explicit ViewLoader(std::vector<uint32_t> nbReleasePyramid)
      : _nbReleasePyramid(std::move(nbReleasePyramid)) {}

  /// \brief Task execution, get the view request, and generate n Tile Request.
  /// \details Get an available empty view from the MemoryManager. Use the
  /// fi::ViewRequestData<UserType> to generate n
  /// fi::TileRequestData<UserType> send to the ATileLoader.
  /// \param viewRequest View request
  void executeTask(
      std::shared_ptr<fi::ViewRequestData<UserType>> viewRequest) {
    if (_nbReleasePyramid[this->getPipelineId()] == 0) {
      return;
    }
    htgs::m_data_t<View<UserType>> viewMemory = ViewLoader<UserType>::template getMemory<View<UserType>>(
            "viewMem",
            new ReleaseCountRule(_nbReleasePyramid[this->getPipelineId()]));
    viewMemory->get()->init(viewRequest);

    uint32_t
        tileHeight = viewRequest->getTileHeight(),
        tileWidth = viewRequest->getTileWidth(),
        minRowFile = viewRequest->getMinRowFile(),
        minColFile = viewRequest->getMinColFile(),
        maxRowFile = viewRequest->getMaxRowFile(),
        maxColFile = viewRequest->getMaxColFile(),
        rowAlreadyFilled = viewRequest->getTopFill(),
        colAlreadyFilled = 0,
        topFill = viewRequest->getTopFill(),
        leftFill = viewRequest->getLeftFill(),
        botFill = viewRequest->getBottomFill(),
        rightFill = viewRequest->getRightFill(),
        rowFrom = 0,
        colFrom = 0,
        heightToCopy = 0,
        widthToCopy = 0,
        ulTileRowGlobal = 0,
        ulTileColGlobal = 0;

    for (uint32_t r = viewRequest->getIndexRowMinTile();
         r < viewRequest->getIndexRowMaxTile(); r++) {
      ulTileRowGlobal = r * tileHeight;
      rowFrom =
          ulTileRowGlobal <= minRowFile ? (minRowFile - ulTileRowGlobal) : 0;
      heightToCopy =
          std::min(maxRowFile, ulTileRowGlobal + tileHeight) - rowFrom
              - ulTileRowGlobal;

      colAlreadyFilled = leftFill;
      for (uint32_t c = viewRequest->getIndexColMinTile();
           c < viewRequest->getIndexColMaxTile(); c++) {
        ulTileColGlobal = c * tileWidth;
        colFrom =
            ulTileColGlobal <= minColFile ? (minColFile - ulTileColGlobal) : 0;
        widthToCopy =
            std::min(maxColFile, ulTileColGlobal + tileWidth) - colFrom
                - ulTileColGlobal;

        // Create the tile request
        auto tileRequestData =
            new fi::TileRequestData<UserType>(r, c, viewMemory,
                                                  viewRequest);
        tileRequestData->setRowFrom(rowFrom);
        tileRequestData->setColFrom(colFrom);
        tileRequestData->setRowDest(rowAlreadyFilled);
        tileRequestData->setColDest(colAlreadyFilled);
        tileRequestData->setHeightToCopy(heightToCopy);
        tileRequestData->setWidthToCopy(widthToCopy);
        tileRequestData->setTopToFill(topFill);
        tileRequestData->setRightToFill(rightFill);
        tileRequestData->setBottomToFill(botFill);
        tileRequestData->setLeftToFill(leftFill);
        this->addResult(tileRequestData);
        colAlreadyFilled += widthToCopy;
      }
      rowAlreadyFilled += heightToCopy;
    }
  }

  /// \brief Get task name
  /// \return Task name
  std::string getName() { return "ViewLoader"; }

  /// \brief Task copy operator
  /// \return New Task
  ViewLoader *copy() { return new ViewLoader(this->_nbReleasePyramid); }

 private:
  std::vector<uint32_t>
      _nbReleasePyramid; ///< Nb of release per level
};
}

#endif //FASTIMAGE_VIEWLOADER_H
