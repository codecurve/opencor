//==============================================================================
// Computer external function class
//==============================================================================

#ifndef COMPUTEREXTERNALFUNCTION_H
#define COMPUTEREXTERNALFUNCTION_H

//==============================================================================

#include <QList>
#include <QString>

//==============================================================================

namespace OpenCOR {
namespace Computer {

//==============================================================================

class ComputerExternalFunction
{
public:
    explicit ComputerExternalFunction(const QString &pName,
                                      const int &pNbOfParameters,
                                      void *pFunction);

    QString name() const;
    int nbOfParameters() const;
    void * function() const;

private:
    QString mName;
    int mNbOfParameters;
    void *mFunction;
};

//==============================================================================

class ComputerExternalFunctions : public QList<ComputerExternalFunction>
{
public:
    bool contains(const ComputerExternalFunction &pExternalFunction) const;
};

//==============================================================================

}   // namespace Computer
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
