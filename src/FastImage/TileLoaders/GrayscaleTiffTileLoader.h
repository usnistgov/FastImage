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

/// @file GrayscaleTiffTileLoader.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/31/17
/// @brief Implements the grayscale tiff tile loader class


#ifndef FASTIMAGE_GRAYSCALETIFFTILELOADER_H
#define FASTIMAGE_GRAYSCALETIFFTILELOADER_H

////Handle type incompatibility between libtiff and openCV in MACOS
#ifdef __APPLE__
#define uint64 uint64_hack_
#define int64 int64_hack_
#include <tiffio.h>
#undef uint64
#undef int64
#else
#include <tiffio.h>
#endif

#include "FastImage/api/ATileLoader.h"
#include "FastImage/data/DataType.h"
#include "FastImage/object/FigCache.h"

namespace fi {
/// \namespace fi FastImage namespace

/**
   * @class GrayscaleTiffTileLoader GrayscaleTiffTileLoader.h <FastImage/TileLoaders/GrayscaleTiffTileLoader.h>
   *
   * @brief Tile loader specialized in grayscale tiff file. Use libtiff library.
   *
   * @details Tile Loader specialized in grayscale tiff images.
   * It opens the file, test if it is grayscale and tiled, and extract tiles from it.
   * Each tile' pixels are converted to the UserType format.
   * It implements the following functions from the ATileLoader:
   * @code
   *  std::string getName() override = 0;
   *  virtual ATileLoader *copyTileLoader() = 0;
   *  virtual uint32_t getImageHeight(uint32_t level = 0) const = 0;
   *  virtual uint32_t getImageWidth(uint32_t level = 0) const = 0;
   *  virtual uint32_t getTileWidth(uint32_t level = 0) const = 0;
   *  virtual uint32_t getTileHeight(uint32_t level = 0) const = 0;
   *  virtual short getBitsPerSample() const = 0;
   *  virtual uint32_t getNbPyramidLevels() const = 0;
   *  virtual double loadTileFromFile(UserType *tile, uint32_t indexRowGlobalTile, uint32_t indexColGlobalTile) = 0;
   * @endcode
   *
   * @tparam UserType Pixel Type asked by the end user
   */
template<typename UserType>
class GrayscaleTiffTileLoader : public ATileLoader<UserType> {

 public:
  /// \brief TiffTileLoader constructor
  /// \details Specialized Tile loader constructor
  /// It opens, and extract metadata from the file thanks to the libtiff library.
  /// These metadata are used to check if the file is tiles and in grayscale.
  /// \param fileName File path
  /// \param numThreads Number of threads used by the tiff tile loader
  explicit GrayscaleTiffTileLoader(const std::string &fileName,
                                   size_t numThreads = 1)
      : ATileLoader<UserType>(fileName,
                              numThreads) {
    short samplesPerPixel = 0;

    // Open the file
    this->_tiff = TIFFOpen(fileName.c_str(), "r");
    if (this->_tiff != nullptr) {
      if (TIFFIsTiled(_tiff) == 0) {
        std::stringstream message;
        message << "Tile Loader ERROR: The image is not tiled.";
        std::string m = message.str();
        throw (FastImageException(m));
      }
      // Load/parse header
      TIFFGetField(this->_tiff, TIFFTAG_IMAGEWIDTH, &(this->_imageWidth));
      TIFFGetField(this->_tiff, TIFFTAG_IMAGELENGTH, &(this->_imageHeight));
      TIFFGetField(this->_tiff, TIFFTAG_TILEWIDTH, &this->_tileWidth);
      TIFFGetField(this->_tiff, TIFFTAG_TILELENGTH, &this->_tileHeight);
      TIFFGetField(this->_tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
      TIFFGetField(this->_tiff, TIFFTAG_BITSPERSAMPLE, &(this->_bitsPerSample));
      TIFFGetField(this->_tiff, TIFFTAG_SAMPLEFORMAT, &(this->_sampleFormat));

      // Test if the file is greyscale
      if (samplesPerPixel != 1) {
        std::stringstream message;
        message
            << "Tile Loader ERROR: The image is not greyscale: "
               "SamplesPerPixel = " << samplesPerPixel << ".";
        std::string m = message.str();
        throw (FastImageException(m));
      }

      // Interpret undefined data format as unsigned integer data
      if (_sampleFormat < 1 && _sampleFormat > 3) {
        _sampleFormat = 1;
      }

    } else {
      std::stringstream message;
      message << "Tile Loader ERROR: The image can not be opened.";
      std::string m = message.str();
      throw (FastImageException(m));
    }
  }

  /// \brief TiffTileLoader default destructor
  /// \details Destroy a GrayscaleTiffTileLoader and close the underlined file
  ~GrayscaleTiffTileLoader() {
    if (this->_tiff != nullptr) {
      TIFFClose(this->_tiff);
    }
  }

  /// \brief Get Image height
  /// \return Image height in px
  uint32_t getImageHeight(uint32_t level = 0) const override {
    return _imageHeight;
  }

  /// \brief Get Image width
  /// \return Image width in px
  uint32_t getImageWidth(uint32_t level = 0) const override {
    return _imageWidth;
  }

  /// \brief Get tile width
  /// \return Tile width in px
  uint32_t getTileWidth(uint32_t level = 0) const override {
    return _tileWidth;
  }

  /// \brief Get tile height
  /// \return Tile height in px
  uint32_t getTileHeight(uint32_t level = 0) const override {
    return _tileHeight;
  }

  /// \brief Get the bits per sample from the file
  /// \details Return the number of bits per sample as defined in the libtiff library:
  /// https://www.awaresystems.be/imaging/tiff/tifftags/bitspersample.html
  /// \return the number of bits per sample
  short getBitsPerSample() const override {
    return _bitsPerSample;
  }

  /// \brief Getter to the number of pyramids levels
  /// \details Get the number of pyramids levels, in this case 1, because the
  /// assumption is the image is planar
  /// \return
  uint32_t getNbPyramidLevels() const override { return 1; }

  /// \brief Loader to the tile with a specific cast
  /// \brief Load a specific tile from the file into a tile buffer,
  /// casting each pixels from  FileType to UserType
  /// \tparam FileType Type deduced from the file
  /// \param src Source data buffer
  /// \param dest Tile buffer
  template<typename FileType>
  void loadTile(tdata_t src, UserType *dest) {
    for (uint32_t row = 0; row < _tileHeight; ++row) {
      for (uint32_t col = 0; col < _tileWidth; ++col) {
        dest[row * _tileWidth + col] =
            (UserType) ((FileType *) (src))[row * _tileWidth + col];
      }
    }
  }

  /// \brief Get down scale Factor for pyramid images. The tiff image for this
  /// loader are not pyramids, so return 1
  /// \param level level to ask--> Not used
  /// \return 1
  float getDownScaleFactor(uint32_t level = 0) override { return 1; }

  /// \brief Load a tile from the disk
  /// \details Allocate a buffer, load a tile from the file into the buffer,
  /// cast each pixel to the right format into parameter tile, and close
  /// the buffer.
  /// \param tile Pointer to a tile already allocated to fill
  /// \param indexRowGlobalTile Row index tile asked
  /// \param indexColGlobalTile Column Index tile asked
  /// \return Duration in mS to load a tile from the disk, use for statistics
  /// purpose
  double loadTileFromFile(UserType *tile,
                          uint32_t indexRowGlobalTile,
                          uint32_t indexColGlobalTile) override {
    tdata_t tiffTile = nullptr;
    tiffTile = _TIFFmalloc(TIFFTileSize(_tiff));
    auto begin = std::chrono::high_resolution_clock::now();
    TIFFReadTile(_tiff,
                 tiffTile,
                 indexColGlobalTile * _tileWidth,
                 indexRowGlobalTile * _tileHeight,
                 0,
                 0);
    auto end = std::chrono::high_resolution_clock::now();
    double diskDuration = (std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - begin).count());
    std::stringstream message;
    switch (this->_sampleFormat) {
      case 1 :
        switch (this->_bitsPerSample) {
          case 8:loadTile<uint8_t>(tiffTile, tile);
            break;
          case 16:loadTile<uint16_t>(tiffTile, tile);
            break;
          case 32:loadTile<uint32_t>(tiffTile, tile);
            break;
          case 64:loadTile<uint64_t>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for "
                   "unsigned integer, number bits per pixel = "
                << this->_bitsPerSample;
            std::string m = message.str();
            throw (FastImageException(m));
        }
        break;
      case 2:
        switch (this->_bitsPerSample) {
          case 8:loadTile<int8_t>(tiffTile, tile);
            break;
          case 16:loadTile<int16_t>(tiffTile, tile);
            break;
          case 32:loadTile<int32_t>(tiffTile, tile);
            break;
          case 64:loadTile<int64_t>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for "
                   "signed integer, number bits per pixel = "
                << this->_bitsPerSample;
            std::string m = message.str();
            throw (FastImageException(m));
        }
        break;
      case 3:
        switch (this->_bitsPerSample) {
          case 8:loadTile<float>(tiffTile, tile);
            break;
          case 16:loadTile<float>(tiffTile, tile);
            break;
          case 32:loadTile<float>(tiffTile, tile);
            break;
          case 64:loadTile<double>(tiffTile, tile);
            break;
          default:
            message
                << "Tile Loader ERROR: The data format is not supported for "
                   "float, number bits per pixel = " << this->_bitsPerSample;
            std::string m = message.str();
            throw (FastImageException(m));
        }
        break;
      default:
        message
            << "Tile Loader ERROR: The data format is not supported, sample "
               "format = " << this->_sampleFormat;
        std::string m = message.str();
        throw (FastImageException(m));
    }

    _TIFFfree(tiffTile);
    return diskDuration;
  }

  /// \brief Copy function used by HTGS to use multiple Tile Loader
  /// \return  A new ATileLoader copied
  ATileLoader<UserType> *copyTileLoader() override {
    return new GrayscaleTiffTileLoader<UserType>(this->getNumThreads(),
                                                 this->getFilePath(),
                                                 *this);
  }

  /// \brief Get the name of the tile loader
  /// \return Name of the tile loader
  std::string getName() override {
    return "TIFF Tile Loader";
  }

 private:
  /// \brief TiffTileLoader constructor used by the copy operator
  /// \param numThreads Number of thread used by the tiff tile loader
  /// \param filePath File path
  /// \param from Origin TiffTileLoader
  GrayscaleTiffTileLoader(size_t numThreads,
                          const std::string &filePath,
                          const GrayscaleTiffTileLoader &from)
      : ATileLoader<UserType>(filePath, numThreads) {
    this->_tiff = TIFFOpen(filePath.c_str(), "r");
    this->_imageWidth = from._imageWidth;
    this->_imageHeight = from._imageHeight;
    this->_tileWidth = from._tileWidth;
    this->_tileHeight = from._tileHeight;
    this->_bitsPerSample = from._bitsPerSample;
    this->_sampleFormat = from._sampleFormat;
  }

  TIFF *
      _tiff = nullptr;             ///< Tiff file pointer

  uint32_t
      _imageHeight = 0,           ///< Image height in pixel
      _imageWidth = 0,            ///< Image width in pixel
      _tileHeight = 0,            ///< Tile height
      _tileWidth = 0;             ///< Tile width

  short
      _sampleFormat = 0,          ///< Sample format as defined by libtiff
      _bitsPerSample = 0;         ///< Bit Per Sample as defined by libtiff

};
}
#endif //FASTIMAGE_GRAYSCALETIFFTILELOADER_H
