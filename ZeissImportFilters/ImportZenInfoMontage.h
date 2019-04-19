/* ============================================================================
 * Copyright (c) 2019-2019 BlueQuartz Software, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the names of any of the BlueQuartz Software contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtXml/QDomDocument>

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/SIMPLib.h"

#include "ZeissImport/ZeissImportDLLExport.h"
#include "ZeissImport/ZeissImportPlugin.h"

// our PIMPL private class
class ImportZenInfoMontagePrivate;

using BoundsType = struct
{
  QString Filename;
  int32_t StartX;
  int32_t SizeX;
  int32_t StartY;
  int32_t SizeY;
  int32_t StartC;
  int32_t StartS;
  int32_t StartB;
  int32_t StartM;
  int32_t Row;
  int32_t Col;
  float SpacingX;
  float SpacingY;
  IDataArray::Pointer ImageDataProxy;
};

/**
 * @brief The ImportZenInfoMontage class. See [Filter documentation](@ref importzeninfomontage) for details.
 */
class ZeissImport_EXPORT ImportZenInfoMontage : public AbstractFilter
{
  Q_OBJECT
  // clang-format off
  PYB11_CREATE_BINDINGS(ImportZenInfoMontage SUPERCLASS AbstractFilter)
  PYB11_PROPERTY(QString InputFile READ getInputFile WRITE setInputFile)
  PYB11_PROPERTY(DataArrayPath DataContainerName READ getDataContainerName WRITE setDataContainerName)
  PYB11_PROPERTY(QString CellAttributeMatrixName READ getCellAttributeMatrixName WRITE setCellAttributeMatrixName)
  PYB11_PROPERTY(QString ImageDataArrayName READ getImageDataArrayName WRITE setImageDataArrayName)
  PYB11_PROPERTY(bool ConvertToGrayScale READ getConvertToGrayScale WRITE setConvertToGrayScale)
  PYB11_PROPERTY(FloatVec3Type ColorWeights READ getColorWeights WRITE setColorWeights)
  PYB11_PROPERTY(bool FileWasRead READ getFileWasRead WRITE setFileWasRead)
  PYB11_PROPERTY(bool ChangeOrigin READ getChangeOrigin WRITE setChangeOrigin)
  PYB11_PROPERTY(FloatVec3Type Origin READ getOrigin WRITE setOrigin)
  PYB11_PROPERTY(bool ChangeSpacing READ getChangeSpacing WRITE setChangeSpacing)
  PYB11_PROPERTY(FloatVec3Type Spacing READ getSpacing WRITE setSpacing)
  PYB11_PROPERTY(int32_t RowCount READ getRowCount)
  PYB11_PROPERTY(int32_t ColumnCount READ getColumnCount)
  PYB11_PROPERTY(QStringList FilenameList READ getFilenameList)

  Q_DECLARE_PRIVATE(ImportZenInfoMontage)

  // clang-format on

public:
  SIMPL_SHARED_POINTERS(ImportZenInfoMontage)
  SIMPL_FILTER_NEW_MACRO(ImportZenInfoMontage)
  SIMPL_TYPE_MACRO_SUPER(ImportZenInfoMontage, AbstractFilter)

  ~ImportZenInfoMontage() override;

  SIMPL_FILTER_PARAMETER(QString, InputFile)
  Q_PROPERTY(QString InputFile READ getInputFile WRITE setInputFile)

  SIMPL_FILTER_PARAMETER(DataArrayPath, DataContainerName)
  Q_PROPERTY(DataArrayPath DataContainerName READ getDataContainerName WRITE setDataContainerName)

  SIMPL_FILTER_PARAMETER(QString, CellAttributeMatrixName)
  Q_PROPERTY(QString CellAttributeMatrixName READ getCellAttributeMatrixName WRITE setCellAttributeMatrixName)

  SIMPL_FILTER_PARAMETER(QString, ImageDataArrayName)
  Q_PROPERTY(QString ImageDataArrayName READ getImageDataArrayName WRITE setImageDataArrayName)

  SIMPL_FILTER_PARAMETER(bool, ConvertToGrayScale)
  Q_PROPERTY(bool ConvertToGrayScale READ getConvertToGrayScale WRITE setConvertToGrayScale)

  SIMPL_FILTER_PARAMETER(bool, ImportAllMetaData)
  Q_PROPERTY(bool ImportAllMetaData READ getImportAllMetaData WRITE setImportAllMetaData)

  SIMPL_FILTER_PARAMETER(FloatVec3Type, ColorWeights)
  Q_PROPERTY(FloatVec3Type ColorWeights READ getColorWeights WRITE setColorWeights)

  SIMPL_INSTANCE_PROPERTY(bool, FileWasRead)
  Q_PROPERTY(bool FileWasRead READ getFileWasRead)

  SIMPL_FILTER_PARAMETER(bool, ChangeOrigin)
  Q_PROPERTY(bool ChangeOrigin READ getChangeOrigin WRITE setChangeOrigin)

  SIMPL_FILTER_PARAMETER(FloatVec3Type, Origin)
  Q_PROPERTY(FloatVec3Type Origin READ getOrigin WRITE setOrigin)

  SIMPL_FILTER_PARAMETER(bool, ChangeSpacing)
  Q_PROPERTY(bool ChangeSpacing READ getChangeSpacing WRITE setChangeSpacing)

  SIMPL_FILTER_PARAMETER(FloatVec3Type, Spacing)
  Q_PROPERTY(FloatVec3Type Spacing READ getSpacing WRITE setSpacing)

  SIMPL_GET_PROPERTY(int32_t, RowCount)
  Q_PROPERTY(int32_t RowCount READ getRowCount)

  SIMPL_GET_PROPERTY(int32_t, ColumnCount)
  Q_PROPERTY(int32_t ColumnCount READ getColumnCount)

  QString getMontageInformation();
  Q_PROPERTY(QString MontageInformation READ getMontageInformation)

  SIMPL_GET_PROPERTY(QStringList, FilenameList)
  Q_PROPERTY(QStringList FilenameList READ getFilenameList)

  /**
   * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
   */
  const QString getCompiledLibraryName() const override;

  /**
   * @brief getBrandingString Returns the branding string for the filter, which is a tag
   * used to denote the filter's association with specific plugins
   * @return Branding string
   */
  const QString getBrandingString() const override;

  /**
   * @brief getFilterVersion Returns a version string for this filter. Default
   * value is an empty string.
   * @return
   */
  const QString getFilterVersion() const override;

  /**
   * @brief newFilterInstance Reimplemented from @see AbstractFilter class
   */
  AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  /**
   * @brief getGroupName Reimplemented from @see AbstractFilter class
   */
  const QString getGroupName() const override;

  /**
   * @brief getSubGroupName Reimplemented from @see AbstractFilter class
   */
  const QString getSubGroupName() const override;

  /**
   * @brief getUuid Return the unique identifier for this filter.
   * @return A QUuid object.
   */
  const QUuid getUuid() override;

  /**
   * @brief getHumanLabel Reimplemented from @see AbstractFilter class
   */
  const QString getHumanLabel() const override;

  /**
   * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
   */
  void setupFilterParameters() override;

  /**
   * @brief execute Reimplemented from @see AbstractFilter class
   */
  void execute() override;

  /**
   * @brief preflight Reimplemented from @see AbstractFilter class
   */
  void preflight() override;

  SIMPL_PIMPL_PROPERTY_DECL(QDomElement, Root)
  SIMPL_PIMPL_PROPERTY_DECL(QString, InputFile_Cache)
  SIMPL_PIMPL_PROPERTY_DECL(QDateTime, TimeStamp_Cache)
  SIMPL_PIMPL_PROPERTY_DECL(std::vector<BoundsType>, BoundsCache)

signals:
  /**
   * @brief updateFilterParameters Emitted when the Filter requests all the latest Filter parameters
   * be pushed from a user-facing control (such as a widget)
   * @param filter Filter instance pointer
   */
  void updateFilterParameters(AbstractFilter* filter);

  /**
   * @brief parametersChanged Emitted when any Filter parameter is changed internally
   */
  void parametersChanged();

  /**
   * @brief preflightAboutToExecute Emitted just before calling dataCheck()
   */
  void preflightAboutToExecute();

  /**
   * @brief preflightExecuted Emitted just after calling dataCheck()
   */
  void preflightExecuted();

protected:
  ImportZenInfoMontage();

  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck();

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

  /**
   * @brief flushCache
   */
  void flushCache();

  /**
   * @brief generateCache
   * @param exportDocument
   */
  void generateCache(QDomElement& exportDocument);

  /**
   * @brief parseImages
   * @param rootTags
   */
  void readImages();

  /**
   * @brief importImage
   * @param dc
   * @param imageFileName
   */
  AbstractFilter::Pointer createImageImportFiler(const QString& imageFileName, const DataArrayPath& daPath);

  /**
   * @brief ImportZenInfoMontage::createColorToGrayScaleFilter
   * @param daPath
   * @return
   */
  AbstractFilter::Pointer createColorToGrayScaleFilter(const DataArrayPath& daPath);

  /**
   * @brief generateDataStructure
   */
  void generateDataStructure();

  /**
   * @brief findTileIndices
   * @param tolerance The tolerance value to group the coordinates by.
   * @param bounds The vector of BoundType objects representing one per tile.
   */
  void findTileIndices(int32_t tolerance, std::vector<BoundsType>& bounds);

private:
  QScopedPointer<ImportZenInfoMontagePrivate> const d_ptr;

  int m_RowCount = -1;
  int m_ColumnCount = -1;
  QStringList m_FilenameList;
  int32_t m_Tolerance = 100;

public:
  /* Rule of 5: All special member functions should be defined if any are defined.
   * https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#c21-if-you-define-or-delete-any-default-operation-define-or-delete-them-all
   */
  ImportZenInfoMontage(const ImportZenInfoMontage&) = delete;            // Copy Constructor Not Implemented
  ImportZenInfoMontage& operator=(const ImportZenInfoMontage&) = delete; // Copy Assignment Not Implemented
  ImportZenInfoMontage(ImportZenInfoMontage&&) = delete;                 // Move Constructor Not Implemented
  ImportZenInfoMontage& operator=(ImportZenInfoMontage&&) = delete;      // Move Assignment Not Implemented
};
