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



/// @file CachedTile.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/25/17
/// @brief Cached tile representation

#ifndef FASTIMAGE_CACHEDTILE_H
#define FASTIMAGE_CACHEDTILE_H

#include <cassert>
#include <ostream>
#include <mutex>

namespace fi {
/// \namespace fi FastImage namespace

/**
 * @class CachedTile CachedTile.h <FastImage/data/CachedTile.h>
 * @brief Tile Cached from the file.
 * @details The cached tile is used to prevent excess IO (i.e. from disk).
 * Once a tile has been loaded from the file it is saved to the cache.
 *
 * @tparam UserType Pixel Type asked by the end user
 *
 */
template<typename UserType>
class CachedTile {
 public:
  /// \brief CachedTile Constructor, allocate the data buffer
  /// \param tileWidth Tile width in px
  /// \param tileHeight Tile height in px
  explicit CachedTile(uint32_t tileWidth, uint32_t tileHeight) :
      _data(new UserType[tileWidth * tileHeight]),
      _indexRow(0),
      _indexCol(0),
      _newTile(true),
      _tileWidth(tileWidth),
      _tileHeight(tileHeight) {}

  /// \brief CachedTile Destructor, deallocate the data buffer.
  ~CachedTile() { delete[] _data; };

  /// \brief Get pointer to the data buffer
  /// \return Pointer to the data buffer
  UserType *getData() const { return _data; }

  /// \brief get row index of the tile in the image
  /// \return Row index of the tile in the image
  uint32_t getIndexRowGlobal() const { return _indexRow; }

  /// \brief get col index of the tile in the image
  /// \return Col index of the tile in the image
  uint32_t getIndexColGlobal() const { return _indexCol; }

  /// \brief Test is the tile is a new tile
  /// \return True if the tile new, else False
  bool isNewTile() const { return _newTile; }

  /// \brief Get the width of the tile
  /// \return Tile width
  uint32_t getTileWidth() const { return _tileWidth; }

  /// \brief Get the height of the tile
  /// \return Tile height
  uint32_t getTileHeight() const { return _tileHeight; }

  /// \brief Setter to column index of the tile in the image
  /// \param indexColGlobal Column index of the tile in the image
  void setIndexColGlobal(uint32_t indexColGlobal) {
    _indexCol = indexColGlobal;
  }

  /// \brief Setter to row index of the tile in the image
  /// \param indexRowGlobal Row index of the tile in the image
  void setIndexRowGlobal(uint32_t indexRowGlobal) {
    _indexRow = indexRowGlobal;
  }

  /// \brief Set if the tile is a new  tile
  /// \param newTile True if the tile is new, else False
  void setNewTile(bool newTile) { _newTile = newTile; }

  /// \brief Lock the tile
  void lock() { _accessMutex.lock(); }

  /// \brief Unlock the tile
  void unlock() { _accessMutex.unlock(); }

  /// \brief Output stream to print a tile
  /// \param os Stream to put the tile information
  /// \param tile The CachedTile to print
  /// \return the output stream with the information
  friend std::ostream &operator<<(std::ostream &os, const CachedTile &tile) {
    os << "CachedTile " << tile._data << "_indexRow: " << tile._indexRow
       << " _indexCol: " << tile._indexCol
       << " _newTile: " << tile._newTile << " _tileWidth: "
       << tile._tileWidth << " _tileHeight: " << tile._tileHeight << std::endl;

    for (uint32_t r = 0; r < tile._tileHeight; ++r) {
      for (uint32_t c = 0; c < tile._tileWidth; ++c) {
        os << tile._data[r * tile._tileWidth + c] << " ";
      }
      os << std::endl;
    }
    os << std::endl;
    return os;
  }
 protected:
  UserType
      *_data;         ///< Tile data.

  uint32_t
      _indexRow,      ///< Row index tile asked in global coordinate
      _indexCol;      ///< Column index tile asked in global coordinate

  bool
      _newTile;       ///< True if the tile is a new tile, else not

  std::mutex
      _accessMutex;   ///< Access mutex

  uint32_t
      _tileWidth,     ///< Tile width
      _tileHeight;    ///< Tile height
};
}

#endif //FASTIMAGE_CACHEDTILE_H
