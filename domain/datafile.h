#ifndef DATAFILE_H
#define DATAFILE_H

#include "file.h"
#include <vector>
#include <QMap>
#include <QDateTime>
#include <complex>
#include <memory>

class Attribute;
class UnivariateCategoryClassification;
class CategoryDefinition;
class IAlgorithmDataSource;

/**
 * @brief The DataFile class is the base class of all project components that are
 *  files with scientific data, namely Point Set and Cartesian Grid.
 * This class basically extends File with a data table and its associated load, save and access methods.
 */

class DataFile : public File
{
public:
    DataFile(QString path);

    /**
      *  Loads the tabular data in file into the _data table.
      */
    void loadData();

    /**
      *  Returns the data at the given position.
      *  This does not follow GEO_EAS convention, so the first data value, at the first line and first column of the file
      *  is at (0,0). ATTENTION: the coordinates are relative to file contents.  Do not confuse with
      *  grid coordinates in regular grids.
      */
    double data(uint line, uint column);

    /**
     * Returns the maximum value in the given column.
     * First column is 0.
     */
    double max( uint column );

    /**
     * Returns the maximum absolute value in the given column.
     * First column is 0.
     */
    double maxAbs( uint column );

    /**
     * Returns the minimum value in the given column.
     * First column is 0.
     */
    double min( uint column );

    /**
     * Returns the minimum absolute value in the given column.
     * First column is 0.
     */
    double minAbs( uint column );

    /**
     * Returns the arithmetic mean of the values in the given column.
     * First column is 0.
     */
    double mean( uint column );

    /**
     * Returns the index of the given field in GEO-EAS convention (first is 1).
     * If the given field name does not exist, returns zero.
     */
    uint getFieldGEOEASIndex( QString field_name );

    /**
     * Returns the Attribute that has the given index in the GEO-EAS file (first is 1).
     * Returns a null pointer of there is no such Attribute.
     */
    Attribute* getAttributeFromGEOEASIndex( uint index );

    /**
     * Returns the GEO-EAS index of the last field of the data file.
     */
    uint getLastFieldGEOEASIndex();

    /**
     * Returns the user-given no-data value text when this data file were added to the project.
     */
    virtual QString getNoDataValue();

    /**
     * Returns the user-given no-data value as a double value.
     * @note This method performs a conversion from the original text value entered by the user.  If it
     * cannot be converted to a double, a generic NaN is returned (std::nan("")).
     */
    virtual double getNoDataValueAsDouble();

    /**
     * Sets the new no-data value.  Empty string means not set.
     */
    virtual void setNoDataValue( const QString new_ndv );

    /**
     * Returns whether the user has given a no-data value when this data file were added to the project.
     */
    virtual bool hasNoDataValue();

    /**
     * Returns whether the given attribute is a declustering weight of another attribute.
     */
    virtual bool isWeight( Attribute* at ) = 0;

    /**
     * Returns whether the given attribute is a normal scores transform of another attribute.
     */
    bool isNormal( Attribute* at );

    /**
     * Returns whether the given attribute is a categorical variable.
     */
    bool isCategorical( Attribute* at );

    /**
     * Returns the category definition associated to the given attribute, supposedly categorical.
     * Returns nullptr if none is found, the attribute is not categorical or does not belong to this data file.
     */
    CategoryDefinition *getCategoryDefinition( Attribute* at );

    /**
     * Returns the variable associated with the declustering weight attribute.
     */
    virtual Attribute* getVariableOfWeight( Attribute* at ) = 0;

    /**
     * Returns the original variable that was normal score transformed into the given attribute.
     */
    Attribute* getVariableOfNScoreVar( Attribute* at );

    /** Replaces the contents of the current physical file with the contents
      * of the given file.
      */
    void replacePhysicalFile( const QString from_file_path );

    /**
     *  Adds a new variable-normal variable relationship in the form given by their
     *  index in the GEO-EAS file.
     * @param trn_file_name Only the file name for the transformation table used to transform
     *                      the original variable.  The file must exist in the project directory.
     */
    void addVariableNScoreVariableRelationship( uint variableGEOEASindex,
                                                uint nScoreVariableGEOEASindex,
                                                QString trn_file_name );

    /**
     * Returns the collections of triads:
     * 1- GEO-EAS index of normal score variable.
     * 2- GEO-EAS index of varibale that was normal score transformed into variable in 1-.
     * 3- File name of the transform table (.trn file).
     */
    QMap<uint, QPair<uint, QString> > getNSVarVarTrnTriads(){ return _nsvar_var_trn; }

    /**
     * Returns the list of GEO-EAS indexes (1st == 1, not zero) of the attributes considered as categorical variables.
     * The second member of the pairs is the name of the category definition file.
     */
    QList< QPair<uint,QString> > getCategoricalAttributes(){ return _categorical_attributes; }

    /**
     * Adds the values stored in an Attribute object as a GEO-EAS column to the given data file.
     * The new column is appended to the end of each of the file's lines.
     * No check is done whether the Attribute has the same value count as data line count in the file.
     * If the Attribute has more values than data lines, the excess values will be ignored.  If there are less velues,
     * the remaining data file lines will be filled with the no-data value: of this DataFile object; if not set, of the
     * source file (Attribute's parent file) or -9999999 if both files do not have a no-data value set.
     * Values of Attribute that are no-data values according to its parent file are rewritten as no-data values
     * according to this DataFile if it has been set; otherwise, -9999999.
     * Of course, after the addition, this object is updated from the changed file contents.
     * @param new_name If empty, the variable name (at->getName()) is used in this file.
     * @param categorical If true, the attribute is handled as a categorical variable.
     * @param cd The pointer to the CategoryDefinition object used to create the categorical attribute, normally
     *           set when the categorical paramater is true.
     * @note This is a file-to-file operation.  To add in-memory data columns, try addDataColumn().
     */
    void addGEOEASColumn(Attribute *at,
                         const QString new_name = "",
                         bool categorical = false,
                         CategoryDefinition *cd = nullptr);

    /**
     * Returns the number of data lines read from file.
     * Make sure to have called loadData() prior to this call, otherwise zero will be returned.
     * Also if you made changes to the data file, it is necessary to call loadData() again to update
     * the object contents.
     */
    uint getDataLineCount();

    /**
     * Returns the number of data columns (variables) of the first line of file (assumes all lines have the
     * same number of columns).
     * Make sure to have called loadData() prior to this call, otherwise zero will be returned.
     * Also if you made changes to the data file, it is necessary to call loadData() again to update
     * the object contents.
     */
    uint getDataColumnCount();

    /** Returns whether the given value equals the no-data value set for this data file.
     * If a no-data value has not been set, this method always returns false.
     * TODO: possible performance bottleneck.
     */
    bool isNDV( double value );

    /**
     * Adds a new data column (variable/attribute) containing categorical values computed from the
     * values of the given variable as a function of the univariate category classification
     * map passed as parameter.  Nothing happens is nullptr is passed as parameter.
     * First column is zero.
     */
    void classify(uint column, UnivariateCategoryClassification* ucc , const QString name_for_new_column);

    /** De-allocates the data loaded with loadData(). */
    void freeLoadedData();

    /** Sets the data page (first and last data line to load).
     * Setting a page, causes a reload in next calls to data() or loadData().  The interval is inclusive,
     * for example, 0 and 2 causes the first three lines of the data file to be loaded, so pay attention when computing
     * data line numbers from grid indexes and realization numbers.
     * To read all data lines in the file, set any interval that will surely include
     * all data lines such as 0 and std::numeric_limits<long>::max().
     * Setting a data page also helps in selecting a realization or range of realizations in Cartesian grids.
     */
    void setDataPage( long firstDataLine, long lastDataLine );

    /** Sets data page to cover the entire file (from line 0 to infinity).
     * @note WARNING! Makes the entire file to be loaded into memory!
     */
    void setDataPageToAll();

    /**
     * Adds the given values in a vector of complex numbers as new or the first two columns of the in-memory data
     * array (_data member variable). New Attribute objects are created to match the newly added data columns.  So,
     * if the data file is saved, the GEO-EAS file will have names for the GEO-EAS data columns.
     * ATTENTION: It is necessary to call File::writeToFS() to commit changes to the filesystem.
     */
    void addDataColumns( std::vector< std::complex<double> >& columns,
                         const QString nameForNewAttributeOfRealPart,
                         const QString nameForNewAttributeOfImaginaryPart);

    /**
     * Returns a pointer to the internal algorithm interface data source (see classes in /algorthms subdirectory).
     */
    IAlgorithmDataSource* algorithmDataSource();

    /**
      * Adds a new data column to the data set filled with the given array of values.
      * The new column will have the same number of data elements of the current columns.  So if the passed array is too short,
      * the remaining data elements will be filled with a default value (zero or NDV).  If the passed array is too long, the
      * exceeding data elements will be ignored. The function returns the column index of the newly added data column.
      * If a CategoryDefinion is passed, then the newly added values will be treated as categorical throughout the system.
      * @note This function cannot be used on DataFile objects created in-code only, that is, without an
      *       associated filesystem file (see File class).  For such cases, see addEmptyDataColumn() [SVD branch].
      */
    int addNewDataColumn( const QString columnName, const std::vector<double> &values, CategoryDefinition *cd = nullptr );

    /** Returns the variance of the values in the given column. */
    double variance( uint column );

    /** Returns the Pearson correlation coefficient of the values in the given columns. */
    double correlation(uint columnX, uint columnY );

//File interface
    void deleteFromFS();
    void writeToFS();

protected:

    /**
     * The data table.  A matrix of doubles.
     * Outer vector are rows of data.
     * Inner vector are values in a row of data.
     */
    std::vector< std::vector<double> > _data;

    /** The no-data value specified by the user. */
    QString _no_data_value;

    /** Repopulates the _children collection.  Mainly useful when there are changes in the physical point set file. */
    void updatePropertyCollection();

    /**
     * pairs relating n-scored variables (first uint) and variables
     * (second uint) by their GEO-EAS indexes (1=first), also
     * the transform table file (QString member) is indicated in the relation
     */
    QMap<uint, QPair<uint, QString> > _nsvar_var_trn;

    /**
     * List of GEO-EAS indexes (1st = 1, not zero) of attributes considered as categorical variables.
     * The second member of the pairs is the name of category definition file.
     */
    QList< QPair<uint, QString> > _categorical_attributes;

    /**
     * Stores the file timestamp in the last call to loadData().
     * This time is used to detect whether there as a change in the file, to prevent
     * unnecessary data reloads.
     */
    QDateTime _lastModifiedDateTimeLastLoad;

    /** The first line of file to load. Default is 0 (first data line). */
    long _dataPageFirstLine;

    /** The last line of file to load.  Default is infinity (read all data). */
    long _dataPageLastLine;

    /** The pointer to the internal interface to the algorithms' data source (see classes in /algorithms subdirectory). */
    std::shared_ptr<IAlgorithmDataSource> _algorithmDataSourceInterface;
};

#endif // DATAFILE_H
