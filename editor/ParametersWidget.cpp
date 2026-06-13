#include "StdAfx.h"
#include "ParametersWidget.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QHeaderView>

CParametersWidget::CParametersWidget(CAnimStateMachineEditor& editor)
	: m_editor(editor)
{
	auto pLayout = new QVBoxLayout(this);

	m_pTree = new QTreeWidget(this);
	m_pTree->setHeaderLabels({ "Name", "Type", "Default" });
	m_pTree->setRootIsDecorated(false);
	pLayout->addWidget(m_pTree);

	auto pAddBtn = new QPushButton("Add Parameter", this);
	pLayout->addWidget(pAddBtn);
}
