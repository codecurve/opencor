//==============================================================================
// Single cell view information parameters widget
//==============================================================================

#include "cellmlfileruntime.h"
#include "propertyeditorwidget.h"
#include "singlecellviewinformationparameterswidget.h"
#include "singlecellviewsimulation.h"

//==============================================================================

#include <QHeaderView>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace SingleCellView {

//==============================================================================

SingleCellViewInformationParametersWidget::SingleCellViewInformationParametersWidget(QWidget *pParent) :
    QStackedWidget(pParent),
    mPropertyEditors(QMap<QString, Core::PropertyEditorWidget *>()),
    mParameters(QMap<Core::Property *, QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> >()),
    mColumnWidths(QList<int>()),
    mSimulationData(0)
{
    // Determine the default width of each column of our property editors

    Core::PropertyEditorWidget *tempPropertyEditor = new Core::PropertyEditorWidget(this);

    for (int i = 0, iMax = tempPropertyEditor->header()->count(); i < iMax; ++i)
        mColumnWidths.append(tempPropertyEditor->columnWidth(i));

    delete tempPropertyEditor;
}

//==============================================================================

void SingleCellViewInformationParametersWidget::retranslateUi()
{
    // Retranslate all our property editors

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        propertyEditor->retranslateUi();

    // Retranslate the tool tip of all our parameters

    updateToolTips();
}

//==============================================================================

static const QString SettingsColumnWidth = "ColumnWidth%1";

//==============================================================================

void SingleCellViewInformationParametersWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve the width of each column of our property editors

    for (int i = 0, iMax = mColumnWidths.size(); i < iMax; ++i)
        mColumnWidths[i] = pSettings->value(SettingsColumnWidth.arg(i),
                                            mColumnWidths.at(i)).toInt();

}

//==============================================================================

void SingleCellViewInformationParametersWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the width of each column of our current property editor

    for (int i = 0, iMax = mColumnWidths.size(); i < iMax; ++i)
        pSettings->setValue(SettingsColumnWidth.arg(i), mColumnWidths.at(i));
}

//==============================================================================

void SingleCellViewInformationParametersWidget::initialize(const QString &pFileName,
                                                           CellMLSupport::CellMLFileRuntime *pRuntime,
                                                           SingleCellViewSimulationData *pSimulationData)
{
    // Keep track of the simulation data
    mSimulationData = pSimulationData;

    // Set the active model name:
    mCellMLFileName = pFileName;


    // If this code is called before we get the initial parameter values from
    // 
    
    // Retrieve the property editor for the given file name or create one, if
    // none exists.
    Core::PropertyEditorWidget *propertyEditor = mPropertyEditors.value(mCellMLFileName);

    if (!propertyEditor) {
        // No property editor exists for the given file name, so create one

        propertyEditor = new Core::PropertyEditorWidget(this);

        // Initialise our property editor's columns' width

        for (int i = 0, iMax = mColumnWidths.size(); i < iMax; ++i)
            propertyEditor->setColumnWidth(i, mColumnWidths.at(i));

        // Populate our property editor

        populateModel(propertyEditor, pRuntime);

        // Keep track of changes to columns' width

        connect(propertyEditor->header(), SIGNAL(sectionResized(int,int,int)),
                this, SLOT(propertyEditorSectionResized(const int &, const int &, const int &)));

        // Keep track of when some of the model's data has changed

        // Keep track of when the user changes a property value

        connect(propertyEditor, SIGNAL(propertyChanged(Core::Property *)),
                this, SLOT(propertyChanged(Core::Property *)));

        // Keep track of when the user wants to show/hide a parameter

        connect(propertyEditor, SIGNAL(propertyChecked(Core::Property *, const bool &)),
                this, SLOT(emitShowParameter(Core::Property *, const bool &)));

        connect(pSimulationData, SIGNAL(updated()),
                this, SLOT(updateParameters()));

        // Add our new property editor to ourselves

        addWidget(propertyEditor);

        // Keep track of our new property editor

        mPropertyEditors.insert(pFileName, propertyEditor);
    }

    // Set our retrieved property editor as our widget

    setCurrentWidget(propertyEditor);

    // Update the tool tip of all our parameters
    // Note: this is in case the user changed the locale and then switched to a
    //       different file...

    updateToolTips();
}

//==============================================================================

void SingleCellViewInformationParametersWidget::finalize(const QString &pFileName)
{
    // Remove any track of our property editor

    mPropertyEditors.remove(pFileName);
}

//==============================================================================

void SingleCellViewInformationParametersWidget::updateParameters()
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    

    // Update our property editor's data

    foreach (Core::Property *property, propertyEditor->properties()) {
        QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> parameter = mParameters.value(property);

        QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter> compiledParameter;
        if (parameter && mSimulationData) {
          compiledParameter = mSimulationData->isDAETypeSolver() ?
            parameter->DAEData() : parameter->ODEData();
        }

        if (compiledParameter)
            switch (compiledParameter->type()) {
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Constant:
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::ComputedConstant:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->constants()[compiledParameter->index()]);

                break;
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::State:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->states()[compiledParameter->index()]);

                break;
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Rate:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->rates()[compiledParameter->index()]);

                break;
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Algebraic:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->algebraic()[compiledParameter->index()]);

                break;
            default:
                // Either Voi or Undefined, so...

                ;
            }
    }

    // Check whether any of our properties has actually been modified

    mSimulationData->checkForModifications();

    // Update the tool tip of all our parameters

    updateToolTips();
}

//==============================================================================

void SingleCellViewInformationParametersWidget::propertyChanged(Core::Property *pProperty)
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Update our simulation data

    QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> parameter = mParameters.value(pProperty);

    QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter> compiledParameter;
    if (parameter && mSimulationData) {
        compiledParameter = mSimulationData->isDAETypeSolver() ?
            parameter->DAEData() : parameter->ODEData();
    }

    if (compiledParameter)
        switch (compiledParameter->type()) {
        case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Constant:
            mSimulationData->constants()[compiledParameter->index()] = propertyEditor->doublePropertyItem(pProperty->value());

            break;
        case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::State:
            mSimulationData->states()[compiledParameter->index()] = propertyEditor->doublePropertyItem(pProperty->value());

            break;
        default:
            // Either Voi, ComputedConstant, Rate, Algebraic or Undefined, so...

            ;
        }

    // Recompute our 'computed constants' and 'variables'
    // Note: we would normally call mSimulationData->checkForModifications()
    //       after recomputing our 'computed constants' and 'variables, but the
    //       recomputation will eventually result in updateParameters() abvove
    //       to be called and it will check for modifications, so...

    mSimulationData->recomputeComputedConstantsAndVariables();
}

//==============================================================================

void SingleCellViewInformationParametersWidget::emitShowParameter(Core::Property *pProperty,
                                                                  const bool &pShow)
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Let people know whether a parameter for the given file name is to be
    // shown

    emit showParameter(mPropertyEditors.key(propertyEditor),
                       mParameters.value(pProperty), pShow);
}

//==============================================================================

void SingleCellViewInformationParametersWidget::finishPropertyEditing()
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Finish the editing of our current property editor

    propertyEditor->finishPropertyEditing();
}

//==============================================================================

void SingleCellViewInformationParametersWidget::populateModel(Core::PropertyEditorWidget *pPropertyEditor,
                                                              CellMLSupport::CellMLFileRuntime *pRuntime)
{
    // Prevent ourselves from being updated (to avoid any flickering)

    pPropertyEditor->setUpdatesEnabled(false);

    // Populate our property editor with the parameters

    Core::Property *section = 0;

    mSimulationData->ensureCodeCompiled();

    foreach (QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> parameter, pRuntime->parameters()) {
        // Check whether the current model parameter is in the same component as
        // the previous one

        QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter> compiledParameter;
        if (parameter && mSimulationData) {
          compiledParameter = mSimulationData->isDAETypeSolver() ?
            parameter->DAEData() : parameter->ODEData();
        }
        if (!compiledParameter)
            continue;

        QString crtComponent = parameter->component();
        if (!section || crtComponent.compare(section->name()->text())) {
            // The current parameter is in a different component, so create a
            // new section for the 'new' component

            section = pPropertyEditor->addSectionProperty();

            pPropertyEditor->setStringPropertyItem(section->name(), crtComponent);
        }

        // Add the current parameter to the 'current' component section
        // Note: in case of an algebraic variable, if its degree is equal to
        //       zero, then we are dealing with a 'proper' algebraic variable
        //       otherwise a rate variable. Now, there may be several rate
        //       variables with the same name (but different degrees) and a
        //       state variable with the same name will also exist. So, to
        //       distinguish between all of them, we 'customise' our variable's
        //       name by appending n single quotes with n the degree of our
        //       variable (that degree is zero for all but rate variables; e.g.
        //       for a state variable which name is V, the corresponding rate
        //       variables of degree 1, 2 and 3 will be V', V'' and V''',
        //       respectively)...

        bool parameterEditable = false;
        QIcon parameterIcon = QIcon(":CellMLSupport_errorNode");

        switch (compiledParameter->type()) {
        case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Constant:
            parameterEditable = true;
            parameterIcon = QIcon(":SingleCellView_constant");

            break;
        case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::ComputedConstant:
            parameterIcon = QIcon(":SingleCellView_computedConstant");

            break;
        case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Rate:
            parameterIcon = QIcon(":SingleCellView_rate");

            break;
        case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::State:
            parameterEditable = true;

            parameterIcon = QIcon(":SingleCellView_state");

            break;
        case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Algebraic:
            parameterIcon = QIcon(":SingleCellView_algebraic");

            break;
        default:
            // We are dealing with a type of parameter which is of no interest
            // to us, so do nothing...
            // Note: we should never reach this point...

            ;
        }

        Core::Property *property = pPropertyEditor->addDoubleProperty(QString(), parameterEditable, true, section);

        property->name()->setIcon(parameterIcon);

        pPropertyEditor->setStringPropertyItem(property->name(), parameter->name()+QString(parameter->degree(), '\''));

        QString perVoiUnitDegree = QString();

        if (parameter->degree()) {
            perVoiUnitDegree += "/"+pRuntime->variableOfIntegration()->unit();

            if (parameter->degree() > 1)
                perVoiUnitDegree += parameter->degree();
        }

        pPropertyEditor->setStringPropertyItem(property->unit(), parameter->unit()+perVoiUnitDegree);

        // Keep track of the link between our property value and parameter

        mParameters.insert(property, parameter);
    }

    // Update the tool tip of all our parameters

    updateToolTips();

    // Expand all our properties

    pPropertyEditor->expandAll();

    // Allow ourselves to be updated again

    pPropertyEditor->setUpdatesEnabled(true);
}

//==============================================================================

void SingleCellViewInformationParametersWidget::updateToolTips()
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Update the tool tip of all our property editor's properties

    foreach (Core::Property *property, propertyEditor->properties()) {
        QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> parameter = mParameters.value(property);

        QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter> compiledParameter;
        if (parameter && mSimulationData) {
          compiledParameter = mSimulationData->isDAETypeSolver() ?
            parameter->DAEData() : parameter->ODEData();
        }
        QString parameterType;
        if (compiledParameter) {

            switch (compiledParameter->type()) {
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Constant:
                parameterType = tr("constant");

                break;
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::ComputedConstant:
                parameterType = tr("computed constant");

                break;
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Rate:
                parameterType = tr("rate");

                break;
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::State:
                parameterType = tr("state");

                break;
            case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Algebraic:
                parameterType = tr("algebraic");

                break;
            default:
                // We are dealing with a type of parameter which is of no
                // interest to us, so do nothing...
                // Note: we should never reach this point...

                ;
            }

            QString parameterToolTip = property->name()->text()+tr(": ")+property->value()->text()+" "+property->unit()->text()+" ("+parameterType+")";

            property->name()->setToolTip(parameterToolTip);
            property->value()->setToolTip(parameterToolTip);
            property->unit()->setToolTip(parameterToolTip);
        }
    }
}

//==============================================================================

void SingleCellViewInformationParametersWidget::propertyEditorSectionResized(const int &pLogicalIndex,
                                                                             const int &pOldSize,
                                                                             const int &pNewSize)
{
    Q_UNUSED(pOldSize);

    // Prevent all our property editors from responding to an updating of their
    // columns' width

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        disconnect(propertyEditor->header(), SIGNAL(sectionResized(int,int,int)),
                   this, SLOT(propertyEditorSectionResized(const int &, const int &, const int &)));

    // Update the column width of all our property editors

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        propertyEditor->header()->resizeSection(pLogicalIndex, pNewSize);

    // Keep track of the new column width

    mColumnWidths[pLogicalIndex] = pNewSize;

    // Re-allow all our property editors to respond to an updating of their
    // columns' width

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        connect(propertyEditor->header(), SIGNAL(sectionResized(int,int,int)),
                this, SLOT(propertyEditorSectionResized(const int &, const int &, const int &)));
}

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
