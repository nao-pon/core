/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <string>

#include <svtools/toolbarmenu.hxx>
#include <svx/colorwindow.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/eitem.hxx>
#include <vcl/settings.hxx>
#include <svl/intitem.hxx>
#include <editeng/colritem.hxx>

#include <svx/strings.hrc>
#include <svx/svdtrans.hxx>
#include <svx/sdasitm.hxx>
#include <svx/dialmgr.hxx>

#include <helpids.h>
#include "extrusioncontrols.hxx"
#include <extrusiondepthdialog.hxx>

#include <bitmaps.hlst>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::graphic;

namespace svx
{

static const sal_Int32 gSkewList[] = { 135, 90, 45, 180, 0, -360, -135, -90, -45 };
static const char g_sExtrusionDirection[] = ".uno:ExtrusionDirection";
static const char g_sExtrusionProjection[] = ".uno:ExtrusionProjection";

static const OUStringLiteral aLightOffBmps[] =
{
    RID_SVXBMP_LIGHT_OFF_FROM_TOP_LEFT,
    RID_SVXBMP_LIGHT_OFF_FROM_TOP,
    RID_SVXBMP_LIGHT_OFF_FROM_TOP_RIGHT,
    RID_SVXBMP_LIGHT_OFF_FROM_LEFT,
    "",
    RID_SVXBMP_LIGHT_OFF_FROM_RIGHT,
    RID_SVXBMP_LIGHT_OFF_FROM_BOTTOM_LEFT,
    RID_SVXBMP_LIGHT_OFF_FROM_BOTTOM,
    RID_SVXBMP_LIGHT_OFF_FROM_BOTTOM_RIGHT
};

static const OUStringLiteral aLightOnBmps[] =
{
    RID_SVXBMP_LIGHT_ON_FROM_TOP_LEFT,
    RID_SVXBMP_LIGHT_ON_FROM_TOP,
    RID_SVXBMP_LIGHT_ON_FROM_TOP_RIGHT,
    RID_SVXBMP_LIGHT_ON_FROM_LEFT,
    "",
    RID_SVXBMP_LIGHT_ON_FROM_RIGHT,
    RID_SVXBMP_LIGHT_ON_FROM_BOTTOM_LEFT,
    RID_SVXBMP_LIGHT_ON_FROM_BOTTOM,
    RID_SVXBMP_LIGHT_ON_FROM_BOTTOM_RIGHT
};

static const OUStringLiteral aLightPreviewBmps[] =
{
    RID_SVXBMP_LIGHT_PREVIEW_FROM_TOP_LEFT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_TOP,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_TOP_RIGHT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_LEFT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_RIGHT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_FRONT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_BOTTOM_LEFT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_BOTTOM,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_BOTTOM_RIGHT
};

static const OUStringLiteral aDirectionBmps[] =
{
    RID_SVXBMP_DIRECTION_DIRECTION_NW,
    RID_SVXBMP_DIRECTION_DIRECTION_N,
    RID_SVXBMP_DIRECTION_DIRECTION_NE,
    RID_SVXBMP_DIRECTION_DIRECTION_W,
    RID_SVXBMP_DIRECTION_DIRECTION_NONE,
    RID_SVXBMP_DIRECTION_DIRECTION_E,
    RID_SVXBMP_DIRECTION_DIRECTION_SW,
    RID_SVXBMP_DIRECTION_DIRECTION_S,
    RID_SVXBMP_DIRECTION_DIRECTION_SE
};

static const char* aDirectionStrs[] =
{
    RID_SVXSTR_DIRECTION_NW,
    RID_SVXSTR_DIRECTION_N,
    RID_SVXSTR_DIRECTION_NE,
    RID_SVXSTR_DIRECTION_W,
    RID_SVXSTR_DIRECTION_NONE,
    RID_SVXSTR_DIRECTION_E,
    RID_SVXSTR_DIRECTION_SW,
    RID_SVXSTR_DIRECTION_S,
    RID_SVXSTR_DIRECTION_SE
};

ExtrusionDirectionWindow::ExtrusionDirectionWindow(
    svt::ToolboxController& rController,
    vcl::Window* pParentWindow
)
    : ToolbarMenu(rController.getFrameInterface(), pParentWindow, WB_STDPOPUP)
    , mrController(rController)
    , maImgPerspective(BitmapEx(RID_SVXBMP_PERSPECTIVE))
    , maImgParallel(BitmapEx(RID_SVXBMP_PARALLEL))
{
    for (sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; ++i)
    {
        maImgDirection[i] = Image(BitmapEx(aDirectionBmps[i]));
    }

    SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectToolbarMenuHdl ) );
    mpDirectionSet = createEmptyValueSetControl();

    mpDirectionSet->SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectValueSetHdl ) );
    mpDirectionSet->SetColCount( 3 );
    mpDirectionSet->EnableFullItemMode( false );

    for (sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; ++i)
    {
        mpDirectionSet->InsertItem(i + 1, maImgDirection[i], SvxResId(aDirectionStrs[i]));
    }

    mpDirectionSet->SetOutputSizePixel(Size(72, 72));

    appendEntry(2, mpDirectionSet );
    appendSeparator();
    appendEntry(0, SvxResId(RID_SVXSTR_PERSPECTIVE), maImgPerspective);
    appendEntry(1, SvxResId(RID_SVXSTR_PARALLEL), maImgParallel);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( g_sExtrusionDirection );
    AddStatusListener( g_sExtrusionProjection );
}

ExtrusionDirectionWindow::~ExtrusionDirectionWindow()
{
    disposeOnce();
}

void ExtrusionDirectionWindow::dispose()
{
    mpDirectionSet.clear();
    ToolbarMenu::dispose();
}

void ExtrusionDirectionWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        for( sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
        {
            mpDirectionSet->SetItemImage( i+1, maImgDirection[ i ] );
        }

        setEntryImage( 0, maImgPerspective );
        setEntryImage( 1, maImgParallel );
    }
}


void ExtrusionDirectionWindow::implSetDirection( sal_Int32 nSkew, bool bEnabled )
{
    if( mpDirectionSet )
    {
        sal_uInt16 nItemId;
        for( nItemId = DIRECTION_NW; nItemId <= DIRECTION_SE; nItemId++ )
        {
            if( gSkewList[nItemId] == nSkew )
                break;
        }

        if( nItemId <= DIRECTION_SE )
        {
            mpDirectionSet->SelectItem( nItemId+1 );
        }
        else
        {
            mpDirectionSet->SetNoSelection();
        }
    }
    enableEntry( 2, bEnabled );
}


void ExtrusionDirectionWindow::implSetProjection( sal_Int32 nProjection, bool bEnabled )
{
    checkEntry( 0, (nProjection == 0) && bEnabled );
    checkEntry( 1, (nProjection == 1 ) && bEnabled );
    enableEntry( 0, bEnabled );
    enableEntry( 1, bEnabled );
}


void ExtrusionDirectionWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
{
    if( Event.FeatureURL.Main == g_sExtrusionDirection )
    {
        if( !Event.IsEnabled )
        {
            implSetDirection( -1, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetDirection( nValue, true );
        }
    }
    else if( Event.FeatureURL.Main == g_sExtrusionProjection )
    {
        if( !Event.IsEnabled )
        {
            implSetProjection( -1, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetProjection( nValue, true );
        }
    }
}


IMPL_LINK( ExtrusionDirectionWindow, SelectValueSetHdl, ValueSet*, pControl, void )
{
    SelectHdl(pControl);
}
IMPL_LINK( ExtrusionDirectionWindow, SelectToolbarMenuHdl, ToolbarMenu*, pControl, void )
{
    SelectHdl(pControl);
}
void ExtrusionDirectionWindow::SelectHdl(void const * pControl)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    if( pControl == mpDirectionSet )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(g_sExtrusionDirection).copy(5);
        aArgs[0].Value <<= gSkewList[mpDirectionSet->GetSelectedItemId()-1];

        mrController.dispatchCommand( g_sExtrusionDirection, aArgs );
    }
    else
    {
        int nProjection = getSelectedEntryId();
        if( (nProjection >= 0) && (nProjection < 2 ) )
        {
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = OUString(g_sExtrusionProjection).copy(5);
            aArgs[0].Value <<= static_cast<sal_Int32>(nProjection);

            mrController.dispatchCommand( g_sExtrusionProjection, aArgs );
            implSetProjection( nProjection, true );
        }
    }
}

ExtrusionDirectionControl::ExtrusionDirectionControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        ".uno:ExtrusionDirectionFloater"
    )
{
}


VclPtr<vcl::Window> ExtrusionDirectionControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionDirectionWindow>::Create( *this, pParent );
}

// XInitialization
void SAL_CALL ExtrusionDirectionControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionDirectionControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionDirectionController" );
}


Sequence< OUString > ExtrusionDirectionControl::getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionDirectionControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionDirectionControl(xContext));
}


ExtrusionDepthDialog::ExtrusionDepthDialog(weld::Window* pParent, double fDepth, FieldUnit eDefaultUnit)
    : GenericDialogController(pParent, "svx/ui/extrustiondepthdialog.ui", "ExtrustionDepthDialog")
    , m_xMtrDepth(m_xBuilder->weld_metric_spin_button("depth", eDefaultUnit))
{
    m_xMtrDepth->set_value(static_cast<int>(fDepth) * 100, FieldUnit::MM_100TH);
}

ExtrusionDepthDialog::~ExtrusionDepthDialog()
{
}

double ExtrusionDepthDialog::getDepth() const
{
    return static_cast<double>(m_xMtrDepth->get_value(FieldUnit::MM_100TH)) / 100.0;
}

double const aDepthListInch[] = { 0, 1270,2540,5080,10160 };
double const aDepthListMM[] = { 0, 1000, 2500, 5000, 10000 };

static const OUStringLiteral gsExtrusionDepth( ".uno:ExtrusionDepth" );
static const OUStringLiteral gsMetricUnit(     ".uno:MetricUnit"     );

ExtrusionDepthWindow::ExtrusionDepthWindow(
    svt::ToolboxController& rController,
    vcl::Window* pParentWindow
)   : ToolbarMenu( rController.getFrameInterface(), pParentWindow, WB_STDPOPUP )
    , mrController( rController )
    , meUnit(FieldUnit::NONE)
    , mfDepth( -1.0 )
{
    SetSelectHdl( LINK( this, ExtrusionDepthWindow, SelectHdl ) );

    Image aImgDepth0(BitmapEx(RID_SVXBMP_DEPTH_0));
    Image aImgDepth1(BitmapEx(RID_SVXBMP_DEPTH_1));
    Image aImgDepth2(BitmapEx(RID_SVXBMP_DEPTH_2));
    Image aImgDepth3(BitmapEx(RID_SVXBMP_DEPTH_3));
    Image aImgDepth4(BitmapEx(RID_SVXBMP_DEPTH_4));
    Image aImgDepthInfinity(BitmapEx(RID_SVXBMP_DEPTH_INFINITY));

    appendEntry(0, "", aImgDepth0);
    appendEntry(1, "", aImgDepth1);
    appendEntry(2, "", aImgDepth2);
    appendEntry(3, "", aImgDepth3);
    appendEntry(4, "", aImgDepth4);
    appendEntry(5, SvxResId(RID_SVXSTR_INFINITY), aImgDepthInfinity);
    appendEntry(6, SvxResId(RID_SVXSTR_CUSTOM));

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( gsExtrusionDepth );
    AddStatusListener( gsMetricUnit );
}

void ExtrusionDepthWindow::implSetDepth( double fDepth )
{
    mfDepth = fDepth;
    int i;
    for( i = 0; i < 7; i++ )
    {
        if( i == 5 )
        {
            checkEntry( i, fDepth >= 338666 );
        }
        else if( i != 6 )
        {
            checkEntry( i, (fDepth == (IsMetric( meUnit ) ? aDepthListMM[i] : aDepthListInch[i]) ) );
        }
    }
}

void ExtrusionDepthWindow::implFillStrings( FieldUnit eUnit )
{
    meUnit = eUnit;

    const char* aDepths[] =
    {
        RID_SVXSTR_DEPTH_0,
        RID_SVXSTR_DEPTH_1,
        RID_SVXSTR_DEPTH_2,
        RID_SVXSTR_DEPTH_3,
        RID_SVXSTR_DEPTH_4
    };

    const char* aDepthsInch[] =
    {
        RID_SVXSTR_DEPTH_0_INCH,
        RID_SVXSTR_DEPTH_1_INCH,
        RID_SVXSTR_DEPTH_2_INCH,
        RID_SVXSTR_DEPTH_3_INCH,
        RID_SVXSTR_DEPTH_4_INCH
    };

    assert(SAL_N_ELEMENTS(aDepths) == SAL_N_ELEMENTS(aDepthsInch));

    const char** pResource = IsMetric(eUnit) ? aDepths : aDepthsInch;

    for (size_t i = 0; i < SAL_N_ELEMENTS(aDepths); ++i)
    {
        OUString aStr(SvxResId(pResource[i]));
        setEntryText(i, aStr);
    }
}

void ExtrusionDepthWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
{
    if( Event.FeatureURL.Main == gsExtrusionDepth )
    {
        if( !Event.IsEnabled )
        {
            implSetDepth( 0 );
        }
        else
        {
            double fValue = 0.0;
            if( Event.State >>= fValue )
                implSetDepth( fValue );
        }
    }
    else if( Event.FeatureURL.Main == gsMetricUnit )
    {
        if( Event.IsEnabled )
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
            {
                implFillStrings( static_cast<FieldUnit>(nValue) );
                if( mfDepth >= 0.0 )
                    implSetDepth( mfDepth );
            }
        }
    }
}

IMPL_LINK_NOARG(ExtrusionDepthWindow, SelectHdl, ToolbarMenu*, void)
{
    int nSelected = getSelectedEntryId();
    if( nSelected != -1 )
    {
        if( nSelected == 6 )
        {
            if ( IsInPopupMode() )
                EndPopupMode();

            const OUString aCommand( ".uno:ExtrusionDepthDialog" );

            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Name = "Depth";
            aArgs[0].Value <<= mfDepth;
            aArgs[1].Name = "Metric";
            aArgs[1].Value <<= static_cast<sal_Int32>( meUnit );

            mrController.dispatchCommand( aCommand, aArgs );
        }
        else
        {
            double fDepth;

            if( nSelected == 5 )
            {
                fDepth = 338666.6;
            }
            else
            {
                fDepth = IsMetric( meUnit ) ? aDepthListMM[nSelected] : aDepthListInch[nSelected];
            }

            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = OUString(gsExtrusionDepth).copy(5);
            aArgs[0].Value <<= fDepth;

            mrController.dispatchCommand( gsExtrusionDepth,  aArgs );
            implSetDepth( fDepth );

            if ( IsInPopupMode() )
                EndPopupMode();
        }
    }
}


// ExtrusionDirectionControl


ExtrusionDepthController::ExtrusionDepthController(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        ".uno:ExtrusionDepthFloater"
    )
{
}


VclPtr<vcl::Window> ExtrusionDepthController::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionDepthWindow>::Create( *this, pParent );
}

// XInitialization
void SAL_CALL ExtrusionDepthController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionDepthController::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionDepthController" );
}


Sequence< OUString > ExtrusionDepthController::getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionDepthController_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionDepthController(xContext));
}


static const char g_sExtrusionLightingDirection[] = ".uno:ExtrusionLightingDirection";
static const char g_sExtrusionLightingIntensity[] = ".uno:ExtrusionLightingIntensity";

ExtrusionLightingWindow::ExtrusionLightingWindow(svt::ToolboxController& rController,
                                                 vcl::Window* pParentWindow)
    : ToolbarMenu(rController.getFrameInterface(), pParentWindow, WB_STDPOPUP)
    , mrController(rController)
    , maImgBright(BitmapEx(RID_SVXBMP_LIGHTING_BRIGHT))
    , maImgNormal(BitmapEx(RID_SVXBMP_LIGHTING_NORMAL))
    , maImgDim(BitmapEx(RID_SVXBMP_LIGHTING_DIM))
    , mnDirection(FROM_FRONT)
    , mbDirectionEnabled(false)
{
    for (sal_uInt16 i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; ++i)
    {
        if( i != FROM_FRONT )
        {
            maImgLightingOff[i] = Image(BitmapEx(aLightOffBmps[i]));
            maImgLightingOn[i] = Image(BitmapEx(aLightOnBmps[i]));
        }
        maImgLightingPreview[i] = Image(BitmapEx(aLightPreviewBmps[i]));
    }

    SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectToolbarMenuHdl ) );

    mpLightingSet = createEmptyValueSetControl();
    mpLightingSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLightingSet->SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectValueSetHdl ) );
    mpLightingSet->SetColCount( 3 );
    mpLightingSet->EnableFullItemMode( false );

    for (sal_uInt16 i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; ++i)
    {
        if( i != FROM_FRONT )
        {
            mpLightingSet->InsertItem( i+1, maImgLightingOff[i] );
        }
        else
        {
            mpLightingSet->InsertItem( 5, maImgLightingPreview[FROM_FRONT] );
        }
    }
    mpLightingSet->SetOutputSizePixel( Size( 72, 72 ) );

    appendEntry(3, mpLightingSet);
    appendSeparator();
    appendEntry(0, SvxResId(RID_SVXSTR_BRIGHT), maImgBright);
    appendEntry(1, SvxResId(RID_SVXSTR_NORMAL), maImgNormal);
    appendEntry(2, SvxResId(RID_SVXSTR_DIM), maImgDim);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( g_sExtrusionLightingDirection );
    AddStatusListener( g_sExtrusionLightingIntensity );
}

ExtrusionLightingWindow::~ExtrusionLightingWindow()
{
    disposeOnce();
}

void ExtrusionLightingWindow::dispose()
{
    mpLightingSet.clear();
    ToolbarMenu::dispose();
}

void ExtrusionLightingWindow::implSetIntensity( int nLevel, bool bEnabled )
{
    for (int i = 0; i < 3; ++i)
    {
        checkEntry( i, (i == nLevel) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

void ExtrusionLightingWindow::implSetDirection( int nDirection, bool bEnabled )
{
    mnDirection = nDirection;
    mbDirectionEnabled = bEnabled;

    if( !bEnabled )
        nDirection = FROM_FRONT;

    sal_uInt16 nItemId;
    for( nItemId = FROM_TOP_LEFT; nItemId <= FROM_BOTTOM_RIGHT; nItemId++ )
    {
        if( nItemId == FROM_FRONT )
        {
            mpLightingSet->SetItemImage( nItemId + 1, maImgLightingPreview[ nDirection ] );
        }
        else
        {
            mpLightingSet->SetItemImage(
                nItemId + 1,
                static_cast<sal_uInt16>(nDirection) == nItemId ? maImgLightingOn[nItemId] : maImgLightingOff[nItemId]
            );
        }
    }

    enableEntry( 3, bEnabled );
}


void ExtrusionLightingWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
{
    if( Event.FeatureURL.Main == g_sExtrusionLightingIntensity )
    {
        if( !Event.IsEnabled )
        {
            implSetIntensity( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetIntensity( nValue, true );
        }
    }
    else if( Event.FeatureURL.Main == g_sExtrusionLightingDirection )
    {
        if( !Event.IsEnabled )
        {
            implSetDirection( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetDirection( nValue, true );
        }
    }
}


void ExtrusionLightingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        implSetDirection( mnDirection, mbDirectionEnabled );
        setEntryImage( 0, maImgBright );
        setEntryImage( 1, maImgNormal );
        setEntryImage( 2, maImgDim    );
    }
}


IMPL_LINK( ExtrusionLightingWindow, SelectValueSetHdl, ValueSet*, pControl, void )
{
    SelectHdl(pControl);
}
IMPL_LINK( ExtrusionLightingWindow, SelectToolbarMenuHdl, ToolbarMenu*, pControl, void )
{
    SelectHdl(pControl);
}
void ExtrusionLightingWindow::SelectHdl(void const * pControl)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    if( pControl == this )
    {
        int nLevel = getSelectedEntryId();
        if( nLevel >= 0 && nLevel != 3 )
        {
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = OUString(g_sExtrusionLightingIntensity).copy(5);
            aArgs[0].Value <<= static_cast<sal_Int32>(nLevel);

            mrController.dispatchCommand( g_sExtrusionLightingIntensity, aArgs );

            implSetIntensity( nLevel, true );
        }
    }
    else
    {
        sal_Int32 nDirection = mpLightingSet->GetSelectedItemId();

        if( (nDirection > 0) && (nDirection < 10) )
        {
            nDirection--;

            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = OUString(g_sExtrusionLightingDirection).copy(5);
            aArgs[0].Value <<= nDirection;

            mrController.dispatchCommand( g_sExtrusionLightingDirection, aArgs );

            implSetDirection( nDirection, true );
        }

    }
}


ExtrusionLightingControl::ExtrusionLightingControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController( rxContext,
                Reference< css::frame::XFrame >(),
                ".uno:ExtrusionDirectionFloater"
    )
{
}


VclPtr<vcl::Window> ExtrusionLightingControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionLightingWindow>::Create( *this, pParent );
}

// XInitialization
void SAL_CALL ExtrusionLightingControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionLightingControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionLightingController" );
}


Sequence< OUString > ExtrusionLightingControl::getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionLightingControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionLightingControl(xContext));
}


static const char g_sExtrusionSurface[] = ".uno:ExtrusionSurface";

ExtrusionSurfaceWindow::ExtrusionSurfaceWindow(
    svt::ToolboxController& rController,
    vcl::Window* pParentWindow)
    : ToolbarMenu(rController.getFrameInterface(), pParentWindow, WB_STDPOPUP)
    , mrController(rController)
{
    SetSelectHdl( LINK( this, ExtrusionSurfaceWindow, SelectHdl ) );

    Image aImgSurface1(BitmapEx(RID_SVXBMP_WIRE_FRAME));
    Image aImgSurface2(BitmapEx(RID_SVXBMP_MATTE));
    Image aImgSurface3(BitmapEx(RID_SVXBMP_PLASTIC));
    Image aImgSurface4(BitmapEx(RID_SVXBMP_METAL));

    appendEntry(0, SvxResId(RID_SVXSTR_WIREFRAME), aImgSurface1);
    appendEntry(1, SvxResId(RID_SVXSTR_MATTE), aImgSurface2);
    appendEntry(2, SvxResId(RID_SVXSTR_PLASTIC), aImgSurface3);
    appendEntry(3, SvxResId(RID_SVXSTR_METAL), aImgSurface4);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( g_sExtrusionSurface );
}

void ExtrusionSurfaceWindow::implSetSurface( int nSurface, bool bEnabled )
{
    for(int i = 0; i < 4; ++i)
    {
        checkEntry( i, (i == nSurface) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

void ExtrusionSurfaceWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
{
    if( Event.FeatureURL.Main == g_sExtrusionSurface )
    {
        if( !Event.IsEnabled )
        {
            implSetSurface( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetSurface( nValue, true );
        }
    }
}


IMPL_LINK_NOARG(ExtrusionSurfaceWindow, SelectHdl, ToolbarMenu*, void)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    sal_Int32 nSurface = getSelectedEntryId();
    if( nSurface >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(g_sExtrusionSurface).copy(5);
        aArgs[0].Value <<= nSurface;

        mrController.dispatchCommand( g_sExtrusionSurface, aArgs );

        implSetSurface( nSurface, true );
    }
}


ExtrusionSurfaceControl::ExtrusionSurfaceControl(
    const Reference< XComponentContext >& rxContext
)
:   svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        ".uno:ExtrusionSurfaceFloater"
    )
{
}


VclPtr<vcl::Window> ExtrusionSurfaceControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionSurfaceWindow>::Create( *this, pParent );
}

// XInitialization
void SAL_CALL ExtrusionSurfaceControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionSurfaceControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionSurfaceController" );
}


Sequence< OUString > ExtrusionSurfaceControl::getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionSurfaceControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionSurfaceControl(xContext));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
