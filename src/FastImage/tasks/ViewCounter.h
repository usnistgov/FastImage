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

/// @file ViewCounter.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/25/17
/// @brief View Counter, graph's last task, finalise the view.

#ifndef FASTIMAGE_VIEWCOUNTER_H
#define FASTIMAGE_VIEWCOUNTER_H

#include <algorithm>

#include <htgs/api/ITask.hpp>
#include "FastImage/data/CachedTile.h"
#include "FastImage/data/TileRequestData.h"

namespace fi {
/// \namespace fi FastImage namespace

/**
  * @class ViewCounter ViewCounter.h <FastImage/tasks/ViewCounter.h>
  *
  * @brief View Counter, graph's last task, finalise the view.
  *
  * @details Finalize the view, wait for each pieces of the view to be loaded
  * from the file. When done, if there is a radius, then a ghost region will
  * be filled in as needed.
  * Insure also the order if the option is set.
  *
  * @tparam UserType Pixel Type asked by the end user
  **/
template<typename UserType>
class ViewCounter : public htgs::ITask<fi::TileRequestData<UserType>,
                                       htgs::MemoryData<fi::View<UserType>>> {
 public:
  /// \brief View counter constructor, set if the view are send in ordered way
  /// or not, and set the filling type.
  explicit ViewCounter(FillingType fillingType = FillingType::FILL,
                       bool ordered = false)
      : _fillingType(fillingType), _ordered(ordered) {}

  /// \brief View counter destructor
  ~ViewCounter() = default;

  /// \brief Get task name
  /// \return Task name
  std::string getName() { return "ViewCounter"; }

  /// \brief Add a traversal to insure ordering.
  void addTraversal(std::queue<std::pair<uint32_t, uint32_t>> traversal) {
    _queueTraversals.push(traversal);
  }

  /// \brief Wait for the view data to be fully loaded from the file data,
  /// Fill the ghost region if needed, send the viewData when completed
  /// \param tileRequestData Tile from the tile loader to test
  void executeTask(
      std::shared_ptr<fi::TileRequestData<UserType>> tileRequestData) {
    auto data = tileRequestData->getViewData();

    // Tile without radius
    if (tileRequestData->getViewRequest()->getNumberTilesToLoad() == 1 ||
        tileRequestData->getViewRequest()->getRadius() == 0) {
      fillGhostRegion(tileRequestData);
      dataReady(data);
      //this->addResult(data);
    } else {
      // Tile with radius
      auto itPos = _countMap.find(data);
      if (itPos != _countMap.end()) {
        // Found pointer, so increment
        (*itPos).second++;
        auto val = (*itPos).second;
        // If all the tiles have been collected for the view, then the view is
        // complete
        if (val == tileRequestData->getViewRequest()->getNumberTilesToLoad()) {
          _countMap.erase(data);
          fillGhostRegion(tileRequestData);
          dataReady(data);
          //this->addResult(data);
        }
      } else {
        // Pointer was not found, so add it in with initial count of 1
        _countMap.
        insert(std::pair<htgs::m_data_t<View<UserType>>, uint32_t>(data, 1));
      }
    }
  }

  /// \brief Copy operator
  /// \return New task
  ViewCounter *copy() { return new ViewCounter(); }

 private:
  /// \brief Filling type where the data are duplicated from the nearest
  /// border :
  /// radius 1 for a 3x3 tile with only ghost region
  ///      a  abc  c
  ///
  ///      a  abc  c
  ///      d  def  f
  ///      g  ghi  i
  ///
  ///      g  ghi  i
  ///
  /// \param tileRequestData
  void fill(
      std::shared_ptr<fi::TileRequestData<UserType>> tileRequestData) {

    UserType *tile = tileRequestData->getViewData()->get()->getData();

    uint32_t
        topFill = tileRequestData->getTopToFill(),
        bottomFill = tileRequestData->getBottomToFill(),
        leftFill = tileRequestData->getLeftToFill(),
        rightFill = tileRequestData->getRightToFill(),
        viewHeight = tileRequestData->getViewHeight(),
        viewWidth = tileRequestData->getViewWidth();

    for (uint32_t row = topFill; row < viewHeight - bottomFill; ++row) {
      // L
      std::fill_n(
          tile + (row * viewWidth),
          leftFill,
          tile[row * viewWidth + leftFill]);
      // R
      std::fill_n(
          tile + (row * viewWidth + viewWidth - rightFill),
          rightFill,
          tile[row * viewWidth + viewWidth - rightFill - 1]);
    }
    for (uint32_t row = 0; row < topFill; ++row) {
      // UL
      std::fill_n(
          tile + (row * viewWidth),
          leftFill,
          tile[topFill * viewWidth + leftFill]);
      // U
      std::copy_n(
          tile + (topFill * viewWidth + leftFill),
          viewWidth - leftFill - rightFill,
          tile + (row * viewWidth + leftFill));
      // UR
      std::fill_n(
          tile + (row * viewWidth + viewWidth - rightFill),
          rightFill,
          tile[row * viewWidth + viewWidth - rightFill - 1]);
    }
    for (uint32_t row = viewHeight - bottomFill; row < viewHeight; ++row) {
      // BL
      std::fill_n(
          tile + (row * viewWidth),
          leftFill,
          tile[(viewHeight - bottomFill - 1) * viewWidth + leftFill]);
      // B
      std::copy_n(
          tile + ((viewHeight - bottomFill - 1) * viewWidth + leftFill),
          viewWidth - leftFill - rightFill,
          tile + (row * viewWidth + leftFill));
      // BR
      std::fill_n(
          tile + (row * viewWidth + viewWidth - rightFill),
          rightFill,
          tile[(viewHeight - bottomFill) * viewWidth - rightFill - 1]);
    }
  }

  /// \brief Switch for the selected filling and fill the ghost region
  /// \param tileRequestData tile to fill
  void fillGhostRegion(
      std::shared_ptr<fi::TileRequestData<UserType>> tileRequestData) {
    switch (_fillingType) {
      case FillingType::FILL:fill(tileRequestData);
        break;
    }
  }

  /// \brief Update the current traversal, used for insure ordering
  void updateCurrentTraversal() {
    if (_currentTraversal.size() == 0 && _queueTraversals.size() > 0) {
      _currentTraversal = _queueTraversals.front();
      _queueTraversals.pop();
    }
  }

  /// \brief Test if the view received is next one to be send if order needed.
  /// \param view View to test
  /// \return True if the view is the next one to be sent, else Fasle
  bool viewIsNext(htgs::m_data_t<fi::View<UserType>> view) {
    return view->get()->getRow() == _currentTraversal.front().first
        && view->get()->getCol() == _currentTraversal.front().second;
  }

  /// \brief Try to send already stored view
  void handleStoredViews() {
    bool elementFound = true;
    std::pair<uint32_t, uint32_t> frontCoord;
    while (elementFound) {
      elementFound = false;
      updateCurrentTraversal();
      frontCoord = _currentTraversal.front();
      for (auto view = _waitingList.begin(); view != _waitingList.end();
           ++view) {
        if (viewIsNext(*view)) {
          this->addResult(*view);
          _waitingList.remove(*view);
          _currentTraversal.pop();
          updateCurrentTraversal();
          elementFound = true;
          break;
        }
      }
    }
  }

  /// \brief Send the current view if no ordered, or handle the traversal and
  /// the views.
  /// \param view View ready to be sent or stored.
  void dataReady(htgs::m_data_t<fi::View<UserType>> view) {
    if (!_ordered) {
      this->addResult(view);
    } else {
      updateCurrentTraversal();
      if (viewIsNext(view)) {
        this->addResult(view);
        _currentTraversal.pop();
        handleStoredViews();
      } else {
        _waitingList.push_back(view);
      }
    }
  }

  FillingType
      _fillingType;   ///< Filling Type to choose the fill

  std::unordered_map<htgs::m_data_t<fi::View<UserType>>, uint32_t>
      _countMap;  ///< Map between the view, and the number of tiles loaded

  std::list<htgs::m_data_t<fi::View<UserType>>>
      _waitingList; ///< Views stored because not in the right order

  std::queue<std::queue<std::pair<uint32_t, uint32_t>>>
      _queueTraversals; ///< List of traversals

  std::queue<std::pair<uint32_t, uint32_t>>
      _currentTraversal; ///< Current traversal

  bool
      _ordered; ///< Order preserved

};
}
#endif //FASTIMAGE_VIEWCOUNTER_H
