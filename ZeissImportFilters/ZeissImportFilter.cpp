/*
 * Your License or Copyright Information can go here
 */

#include "ZeissImportFilter.h"

#include <string.h>

#include <set>

#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QTextStream>

#include "DREAM3DLib/Common/FilterManager.h"
#include "DREAM3DLib/Common/IFilterFactory.hpp"

#include "ZeissImport/ZeissImportConstants.h"
#include "ZeissImport/ZeissXml/ZeissTagMapping.h"

#define ZIF_PRINT_DBG_MSGS 0


static const QString k_DataContaineNameDefaultName("Zeiss Axio Vision Montage");
static const QString k_TileAttributeMatrixDefaultName("Tile AttributeMatrix");
static const QString k_GrayScaleTempArrayName("gray_scale_temp");

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ZeissImportFilter::ZeissImportFilter() :
  AbstractFilter(),
  m_InputFile(""),
  m_DataContainerName(k_DataContaineNameDefaultName),
  m_ImageDataArrayPrefix(DREAM3D::CellData::ImageData),
  m_ImageAttributeMatrixName(k_TileAttributeMatrixDefaultName)
{
  m_ColorWeights.x = 0.2125f;
  m_ColorWeights.y = 0.7154f;
  m_ColorWeights.z = 0.0721f;
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ZeissImportFilter::~ZeissImportFilter()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FileSystemFilterParameter::New("Input File", "InputFile", FilterParameterWidgetType::InputFileWidget, getInputFile(), false, "", "*.xml"));
  parameters.push_back(FilterParameter::New("DataContainer Name", "DataContainerName", FilterParameterWidgetType::StringWidget, getDataContainerName(), false));
  parameters.push_back(FilterParameter::New("Attribute Matrix Name", "ImageAttributeMatrixName", FilterParameterWidgetType::StringWidget, getImageAttributeMatrixName(), false));

  QStringList linkedProps("ColorWeights");
  parameters.push_back(LinkedBooleanFilterParameter::New("Convert To GrayScale", "ConvertToGrayScale", getConvertToGrayScale(), linkedProps, false));
  parameters.push_back(FilterParameter::New("Color Weighting", "ColorWeights", FilterParameterWidgetType::FloatVec3Widget, getColorWeights(), false));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setInputFile(reader->readString("InputFile", getInputFile() ) );
  setDataContainerName(reader->readString("DataContainerName", getDataContainerName() ) );
  setImageAttributeMatrixName(reader->readString("ImageAttributeMatrixName", getImageAttributeMatrixName() ) );
  setImageDataArrayPrefix(reader->readString("ImageDataArrayPrefix", getImageDataArrayPrefix() ) );
  setConvertToGrayScale(reader->readValue("ConvertToGrayScale", getConvertToGrayScale() ) );
  setColorWeights( reader->readFloatVec3("ColorWeights", getColorWeights() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ZeissImportFilter::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(InputFile)
  DREAM3D_FILTER_WRITE_PARAMETER(DataContainerName)
  DREAM3D_FILTER_WRITE_PARAMETER(ImageAttributeMatrixName)
  DREAM3D_FILTER_WRITE_PARAMETER(ImageDataArrayPrefix)
  DREAM3D_FILTER_WRITE_PARAMETER(ConvertToGrayScale)
  DREAM3D_FILTER_WRITE_PARAMETER(ColorWeights)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::dataCheck()
{
  setErrorCondition(0);

  QString ss;
  QFileInfo fi(getInputFile());
  if (getInputFile().isEmpty() == true)
  {
    ss = QObject::tr("%1 needs the Input File Set and it was not.").arg(ClassName());
    setErrorCondition(-387);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }
  else if (fi.exists() == false)
  {
    ss = QObject::tr("The input file does not exist.");
    setErrorCondition(-388);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  if(getErrorCondition() < 0) { return; }

  DataContainerArray::Pointer dca = getDataContainerArray();
  if(NULL == dca.get())
  {
    ss = QObject::tr("%1 needs a valid DataContainerArray").arg(ClassName());
    setErrorCondition(-390);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
  VolumeDataContainer* m = dca->createNonPrereqDataContainer<VolumeDataContainer, AbstractFilter>(this, getDataContainerName());
  if(getErrorCondition() < 0) { return; }

  // Parse the XML file to get all the meta-data information and create all the
  // data structure that is needed.
  QFile xmlFile(getInputFile());
  int success = readMetaXml(&xmlFile);
  if(success < 0)
  {
    ss = QObject::tr("Could not parse Zeiss XML file");
    setErrorCondition(-389);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::preflight()
{
  // These are the REQUIRED lines of CODE to make sure the filter behaves correctly
  setInPreflight(true); // Set the fact that we are preflighting.
  emit preflightAboutToExecute(); // Emit this signal so that other widgets can do one file update
  emit updateFilterParameters(this); // Emit this signal to have the widgets push their values down to the filter
  dataCheck(); // Run our DataCheck to make sure everthing is setup correctly
  emit preflightExecuted(); // We are done preflighting this filter
  setInPreflight(false); // Inform the system this filter is NOT in preflight mode anymore.
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::execute()
{
  int err = 0;
  // typically run your dataCheck function to make sure you can get that far and all your variables are initialized
  dataCheck();
  // Check to make sure you made it through the data check. Errors would have been reported already so if something
  // happens to fail in the dataCheck() then we simply return
  if(getErrorCondition() < 0) { return; }
  setErrorCondition(0);

  /* If some error occurs this code snippet can report the error up the call chain*/
  if (err < 0)
  {
    QString ss = QObject::tr("Error Importing a Zeiss AxioVision file set.");
    setErrorCondition(-90000);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ZeissImportFilter::readMetaXml(QIODevice* device)
{
  int err = 0;
  QDomDocument domDocument;
  QString errorStr;
  int errorLine;
  int errorColumn;
  if (!domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn))   {
    QString ss = QObject::tr("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
    setErrorCondition(-70000);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return -1;
  }
  QDomElement root = domDocument.documentElement();

  QDomElement tags = root.firstChildElement(ZeissImport::Xml::Tags);
  if (tags.isNull() == true)
  {
    QString ss = QObject::tr("Could not find the <ROOT><Tags> element. Aborting Parsing. Is the file a Zeiss _meta.xml file");
    setErrorCondition(-70001);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return -1;
  }

  // First parse the <ROOT><Tags> section to get the values of how many images we are going to have
  ZeissTagsXmlSection::Pointer rootTagsSection = parseTagsSection(tags);
  if(NULL == rootTagsSection.get() )
  {
    return -1;
  }

  // Now parse each of the <pXXX> tags
  parseImages(root, rootTagsSection);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ZeissTagsXmlSection::Pointer ZeissImportFilter::parseTagsSection(QDomElement& tags)
{
  int count = -1;
  bool ok = false;
  // qDebug() << tags.nodeName() << " node type: " << tags.nodeType();

  QDomElement countEle = tags.firstChildElement(ZeissImport::Xml::Count);

  count = countEle.text().toInt(&ok, 10);
  if(!ok)
  {
    QString ss = QObject::tr("Error Parsing 'Count' Tag in Root 'Tags' DOM element");
    setErrorCondition(-70001);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return ZeissTagsXmlSection::NullPointer();
  }

  ZeissTagsXmlSection::Pointer rootTagsSection = ZeissTagsXmlSection::New();
  ZeissTagMapping::Pointer tagMapping = ZeissTagMapping::instance();

  std::set<int> unknownTags;

  for(int c = 0; c < count; c++)
  {
    QString Vx = QString("V%1").arg(c);
    QString Ix = QString("I%1").arg(c);
    QString Ax = QString("A%1").arg(c);



    QDomElement vxEle = tags.firstChildElement(Vx);
    QDomElement ixEle = tags.firstChildElement(Ix);

 //   qDebug() << Ix << " " << ixEle.text() << "   " << Vx << " " << vxEle.text();

    qint32 idValue =  ixEle.text().toInt(&ok, 10);
    AbstractZeissMetaData::Pointer ptr = ZeissTagMapping::instance()->metaDataForId(idValue, vxEle.text());
    if (NULL != ptr.get() && vxEle.text().size() > 0)
    {
      rootTagsSection->addMetaDataEntry(ptr);
    }
#if ZIF_PRINT_DBG_MSGS
    else
    {
      unknownTags.insert(idValue);
      //      QString str;
      //      QTextStream ss(&str);
      //      ss << "<" << Ix << ">" << idValue << "</" << Ix << "> is Unknown to the Tag Mapping Software";
      //      qDebug() << str;
    }
#endif
  }

#if ZIF_PRINT_DBG_MSGS

  if(unknownTags.size() > 0)
  {
    QString str;
    QTextStream ss(&str);
    ss << "======= Unknown Zeiss Axio Vision _Meta XML Tags ===================\n";
    for(std::set<int>::iterator iter = unknownTags.begin(); iter != unknownTags.end(); ++iter)
    {
      ss <<  *iter << " is Unknown to the Tag Mapping Software\n";
    }
    qDebug() << str;
  }
#endif

  return rootTagsSection;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::parseImages(QDomElement& root, ZeissTagsXmlSection::Pointer rootTagsSection)
{
  AbstractZeissMetaData::Pointer ptr = rootTagsSection->getEntry(Zeiss::MetaXML::ImageCountRawId);

  Int32ZeissMetaEntry::Pointer imageCountPtr = ZeissMetaEntry::convert<Int32ZeissMetaEntry>(ptr);
  Q_ASSERT_X(imageCountPtr.get() != NULL, "Could not Cast to Int32ZeissMetaEntry", "");

  qint32 imageCount = imageCountPtr->getValue();

  ptr = rootTagsSection->getEntry(Zeiss::MetaXML::FilenameId);
  StringZeissMetaEntry::Pointer imageNamePtr = boost::dynamic_pointer_cast<StringZeissMetaEntry>(ptr);
  QString imageName = imageNamePtr->getValue();

  int zeroPadding = 0;
  if(imageCount > 0) { zeroPadding++; }
  if(imageCount > 9) { zeroPadding++; }
  if(imageCount > 99) { zeroPadding++; }
  if(imageCount > 999) { zeroPadding++; }
  if(imageCount > 9999) { zeroPadding++; }


  DataContainerArray::Pointer dca = getDataContainerArray();

  DataContainer::Pointer dc = dca->getDataContainer(getDataContainerName());

  AttributeMatrix::Pointer tileAm = AttributeMatrix::NullPointer();
  AttributeMatrix::Pointer metaAm = AttributeMatrix::NullPointer();

  for(int p = 0; p < imageCount; p++)
  {

    // Generate the xml tag that is for this image
    QString pTag;
    QTextStream out(&pTag);
    out << "p";
    out.setFieldWidth(zeroPadding);
    out.setFieldAlignment(QTextStream::AlignRight);
    out.setPadChar('0');
    out << p;

    // Send a status update on the progress
    QString msg = QString("%1: Importing file %2 of %3").arg(getHumanLabel()).arg(p).arg(imageCount);
    notifyStatusMessage(getHumanLabel(), msg);


    QDomElement photoEle = root.firstChildElement(pTag);
    if(photoEle.isNull())
    {
      QString ss = QObject::tr("Could not find the <ROOT><%1> element. Aborting Parsing. Is the file a Zeiss _meta.xml file").arg(pTag);
      setErrorCondition(-70002);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      return;
    }
    QDomElement tags = photoEle.firstChildElement(ZeissImport::Xml::Tags);
    if (tags.isNull() == true)
    {
      QString ss = QObject::tr("Could not find the <ROOT><%1><Tags> element. Aborting Parsing. Is the file a Zeiss _meta.xml file").arg(pTag);
      setErrorCondition(-70003);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      return;
    }


    ZeissTagsXmlSection::Pointer photoTagsSection = parseTagsSection(tags);
    if(NULL == photoTagsSection.get() )
    {
      QString ss = QObject::tr("Error Parsing the <ROOT><%1><Tags> element. Aborting Parsing. Is the file a Zeiss _meta.xml file").arg(pTag);
      setErrorCondition(-70004);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      return;
    }

    //Get the list of Meta Data Array Names that we are going to parse
//    if(p == 0)
//    {
//      QStringList metaDataArrayNames;
//      ZeissTagMapping::Pointer tagMap = ZeissTagMapping::instance();

//      ZeissTagsXmlSection::MetaDataType metaDataMap = photoTagsSection->getMetaDataMap();
//      QMapIterator<int, AbstractZeissMetaData::Pointer> iter(metaDataMap);
//      while(iter.hasNext())
//      {
//        iter.next();
//        QString tagName = tagMap->nameForId(iter.value()->getZeissIdTag());
//        metaDataArrayNames.append(tagName);
//      }
//    }

    AbstractZeissMetaData::Pointer ptr = photoTagsSection->getEntry(Zeiss::MetaXML::ImageTileIndexId);

    Int32ZeissMetaEntry::Pointer int32Entry = ZeissMetaEntry::convert<Int32ZeissMetaEntry>(ptr);
    Q_ASSERT_X(int32Entry.get() != NULL, "Could not Cast to Int32ZeissMetaEntry", "");

    if(p == 0)
    {
      QVector<size_t> dims = getImageDimensions(photoTagsSection);
      tileAm = dc->createAndAddAttributeMatrix(dims, getImageAttributeMatrixName(), DREAM3D::AttributeMatrixType::Generic);

      dims.resize(1);
      dims[0] = imageCount;
      QString metaName = getImageAttributeMatrixName() + DREAM3D::StringConstants::MetaData;
      metaAm = dc->createAndAddAttributeMatrix(dims, metaName, DREAM3D::AttributeMatrixType::Generic);
      ZeissTagsXmlSection::MetaDataType tagMap = photoTagsSection->getMetaDataMap();
      QMapIterator<int, AbstractZeissMetaData::Pointer> iter(tagMap);
      while(iter.hasNext())
      {
        iter.next();
        IDataArray::Pointer dataArray = iter.value()->createDataArray(!getInPreflight());
        dataArray->resize(imageCount);
        metaAm->addAttributeArray(dataArray->getName(), dataArray);
      }
    }
    // Generate all the Meta Data Values:
    addMetaData(metaAm, photoTagsSection, p);

    // Read the image into a data array
    importImage(imageName, pTag, dc->getName());
    if(getConvertToGrayScale())
    {
      convertToGrayScale(imageName, pTag, dc->getName());
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::addMetaData(AttributeMatrix::Pointer metaAm, ZeissTagsXmlSection::Pointer photoTagsSection, int index)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ZeissImportFilter::importImage(const QString &imageName, const QString &pTag, const QString &dcName)
{

  QFileInfo fi(imageName);
  QString imagePath = fi.completeBaseName() + "_" + pTag + "." + fi.suffix();
  QString dataArrayName = fi.completeBaseName() + "_" + pTag;

  fi = QFileInfo(getInputFile());
  imagePath = fi.absoluteDir().path() + "/" + imagePath;
  //   std::string sPath = imagePath.toStdString();

  QString filtName = "ReadImage";
  FilterManager* fm = FilterManager::Instance();
  IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);
  if (NULL != filterFactory.get() )
  {
    // If we get this far, the Factory is good so creating the filter should not fail unless something has
    // horribly gone wrong in which case the system is going to come down quickly after this.
    AbstractFilter::Pointer filter = filterFactory->create();

    // Connect up the Error/Warning/Progress object so the filter can report those things
    connect(filter.get(), SIGNAL(filterGeneratedMessage(const PipelineMessage&)),
            this, SLOT(broadcastPipelineMessage(const PipelineMessage&)));
    filter->setDataContainerArray(getDataContainerArray()); // AbstractFilter implements this so no problem

    bool propWasSet = filter->setProperty("InputFileName", imagePath);
    if(false == propWasSet)
    {
      QString ss = QObject::tr("Error Setting Property '%1' into filter '%2' which is a subfilter called by %3. The property was not set which could mean the property was not exposed with a Q_PROPERTY macro. Please notify the developers.")
          .arg("InputFileName").arg(filtName).arg(getHumanLabel());
      setErrorCondition(-70005);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }

    propWasSet = filter->setProperty("DataContainerName", dcName);
    if(false == propWasSet)
    {
      QString ss = QObject::tr("Error Setting Property '%1' into filter '%2' which is a subfilter called by %3. The property was not set which could mean the property was not exposed with a Q_PROPERTY macro. Please notify the developers.")
          .arg("DataContainerName").arg(filtName).arg(getHumanLabel());
      setErrorCondition(-70006);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }
    propWasSet = filter->setProperty("CellAttributeMatrixName", getImageAttributeMatrixName());
    if(false == propWasSet)
    {
      QString ss = QObject::tr("Error Setting Property '%1' into filter '%2' which is a subfilter called by %3. The property was not set which could mean the property was not exposed with a Q_PROPERTY macro. Please notify the developers.")
          .arg("CellAttributeMatrixName").arg(filtName).arg(getHumanLabel());
      setErrorCondition(-70007);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }
    propWasSet = filter->setProperty("ImageDataArrayName", dataArrayName);
    if(false == propWasSet)
    {
      QString ss = QObject::tr("Error Setting Property '%1' into filter '%2' which is a subfilter called by %3. The property was not set which could mean the property was not exposed with a Q_PROPERTY macro. Please notify the developers.")
      .arg("ImageDataArrayName").arg(filtName).arg(getHumanLabel());
      setErrorCondition(-70008);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }
    if(getInPreflight() == true)
    {
      filter->preflight();
    }
    else
    {
      filter->execute();
    }

  }
  else {
    QString ss = QObject::tr("Error trying to instantiate the 'ReadImage' filter which is typically included in the 'ImageProcessing' plugin.");
    setErrorCondition(-70009);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

}


// -----------------------------------------------------------------------------
//
// -------------------------------------------------------------------------
void ZeissImportFilter::convertToGrayScale(const QString &imageName, const QString &pTag, const QString &dcName)
{


  QFileInfo fi(imageName);
  QString dataArrayName = fi.completeBaseName() + "_" + pTag;



  QString filtName = "RGBToGray";
  FilterManager* fm = FilterManager::Instance();
  IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);
  if (NULL != filterFactory.get() )
  {
    // If we get this far, the Factory is good so creating the filter should not fail unless something has
    // horribly gone wrong in which case the system is going to come down quickly after this.
    AbstractFilter::Pointer filter = filterFactory->create();

    // Connect up the Error/Warning/Progress object so the filter can report those things
    connect(filter.get(), SIGNAL(filterGeneratedMessage(const PipelineMessage&)),
            this, SLOT(broadcastPipelineMessage(const PipelineMessage&)));
    filter->setDataContainerArray(getDataContainerArray()); // AbstractFilter implements this so no problem

    DataArrayPath dap(getDataContainerName(), getImageAttributeMatrixName(), dataArrayName);

    QVariant variant;
    variant.setValue(dap);
    bool propWasSet = filter->setProperty("SelectedCellArrayArrayPath", variant);
    if(false == propWasSet)
    {
      QString ss = QObject::tr("Error Setting Property '%1' into filter '%2' which is a subfilter called by %3. The property was not set which could mean the property was not exposed with a Q_PROPERTY macro. Please notify the developers.")
          .arg("InputFileName").arg(filtName).arg(getHumanLabel());
      setErrorCondition(-70005);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }

    variant.setValue(getColorWeights());
    propWasSet = filter->setProperty("ColorWeights", variant);
    if(false == propWasSet)
    {
      QString ss = QObject::tr("Error Setting Property '%1' into filter '%2' which is a subfilter called by %3. The property was not set which could mean the property was not exposed with a Q_PROPERTY macro. Please notify the developers.")
          .arg("DataContainerName").arg(filtName).arg(getHumanLabel());
      setErrorCondition(-70006);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }
    propWasSet = filter->setProperty("NewCellArrayName", k_GrayScaleTempArrayName);
    if(false == propWasSet)
    {
      QString ss = QObject::tr("Error Setting Property '%1' into filter '%2' which is a subfilter called by %3. The property was not set which could mean the property was not exposed with a Q_PROPERTY macro. Please notify the developers.")
          .arg("CellAttributeMatrixName").arg(filtName).arg(getHumanLabel());
      setErrorCondition(-70007);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }
    if(getInPreflight() == true)
    {
      filter->preflight();
    }
    else
    {
      filter->execute();
    }

    DataContainerArray::Pointer dca = getDataContainerArray();
    DataContainer::Pointer dc = dca->getDataContainer(getDataContainerName());
    AttributeMatrix::Pointer am = dc->getAttributeMatrix(getImageAttributeMatrixName());
    IDataArray::Pointer rgb = am->removeAttributeArray(dataArrayName);
    IDataArray::Pointer gray = am->removeAttributeArray(k_GrayScaleTempArrayName);
    gray->setName(rgb->getName());
    am->addAttributeArray(gray->getName(), gray);
  }
  else {
    QString ss = QObject::tr("Error trying to instantiate the 'ReadImage' filter which is typically included in the 'ImageProcessing' plugin.");
    setErrorCondition(-70009);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<size_t> ZeissImportFilter::getImageDimensions(ZeissTagsXmlSection::Pointer photoTagsSection)
{
  QVector<size_t> dims(3);
  AbstractZeissMetaData::Pointer ptr = photoTagsSection->getEntry(Zeiss::MetaXML::ImageWidthPixelId);
  Int32ZeissMetaEntry::Pointer int32Entry = ZeissMetaEntry::convert<Int32ZeissMetaEntry>(ptr);
  Q_ASSERT_X(int32Entry.get() != NULL, "Could not Cast to Int32ZeissMetaEntry", "");

  dims[0] = int32Entry->getValue();

  ptr = photoTagsSection->getEntry(Zeiss::MetaXML::ImageHeightPixelId);
  int32Entry = ZeissMetaEntry::convert<Int32ZeissMetaEntry>(ptr);
  Q_ASSERT_X(int32Entry.get() != NULL, "Could not Cast to Int32ZeissMetaEntry", "");

  dims[1] = int32Entry->getValue();
  dims[2] = 1;
  return dims;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ZeissImportFilter::getCompiledLibraryName()
{
  return ZeissImport::ZeissImportBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ZeissImportFilter::getGroupName()
{
  return "ZeissImport";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ZeissImportFilter::getHumanLabel()
{
  return "Zeiss AxioVision Import";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ZeissImportFilter::getSubGroupName()
{
  return "IO";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ZeissImportFilter::newFilterInstance(bool copyFilterParameters)
{
  /*
  * write code to optionally copy the filter parameters from the current filter into the new instance
  */
  ZeissImportFilter::Pointer filter = ZeissImportFilter::New();
  if(true == copyFilterParameters)
  {
    /* If the filter uses all the standard Filter Parameter Widgets you can probabaly get
     * away with using this method to copy the filter parameters from the current instance
     * into the new instance
     */
    copyFilterParameterInstanceVariables(filter.get());
    /* If your filter is using a lot of custom FilterParameterWidgets @see ReadH5Ebsd then you
     * may need to copy each filter parameter explicitly plus any other instance variables that
     * are needed into the new instance. Here is some example code from ReadH5Ebsd
     */
    //    DREAM3D_COPY_INSTANCEVAR(OutputFile)
    //    DREAM3D_COPY_INSTANCEVAR(ZStartIndex)
    //    DREAM3D_COPY_INSTANCEVAR(ZEndIndex)
    //    DREAM3D_COPY_INSTANCEVAR(ZResolution)
  }
  return filter;
}

