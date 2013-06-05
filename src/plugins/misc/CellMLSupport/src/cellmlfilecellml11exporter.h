//==============================================================================
// CellML file CellML 1.1 exporter
//==============================================================================

#ifndef CELLMLFILECELLML11EXPORTER_H
#define CELLMLFILECELLML11EXPORTER_H

//==============================================================================

#include "cellmlfilecellmlexporter.h"

//==============================================================================

#include <QString>

//==============================================================================

namespace OpenCOR {
namespace CellMLSupport {

//==============================================================================

class CellMLFileCellML11Exporter : public CellMLFileCellMLExporter
{
public:
    explicit CellMLFileCellML11Exporter(iface::cellml_api::Model *pModel,
                                        const QString &pFileName);
};

//==============================================================================

}   // namespace CellMLSupport
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
