#include "StdAfx.h"
#include "PreviewWidget.h"
#include <QVBoxLayout>
#include <QLabel>

CPreviewWidget::CPreviewWidget(CAnimStateMachineEditor& editor)
	: m_editor(editor)
{
	auto pLayout = new QVBoxLayout(this);
	auto pLabel = new QLabel("Animation Preview", this);
	pLabel->setAlignment(Qt::AlignCenter);
	pLayout->addWidget(pLabel);
}
