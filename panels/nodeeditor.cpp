#include "nodeeditor.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QDebug>

NodeEditor::NodeEditor(QWidget *parent) :
  EffectsPanel(parent),
  view_(&scene_)
{
  setWindowTitle(tr("Node Editor"));
  resize(720, 480);

  QWidget* central_widget = new QWidget();
  setWidget(central_widget);

  QVBoxLayout* layout = new QVBoxLayout(central_widget);
  layout->setSpacing(0);
  layout->setMargin(0);

  layout->addWidget(&view_);

  view_.setInteractive(true);
  view_.setDragMode(QGraphicsView::RubberBandDrag);

  connect(&scene_, SIGNAL(changed(const QList<QRectF>&)), this, SLOT(ItemsChanged()));
}

void NodeEditor::Retranslate()
{
}

void NodeEditor::LoadEvent()
{
  if (!open_effects_.isEmpty()) {
    QVector<NodeEdge*> added_edges;

    Clip* first_clip = open_effects_.first()->GetEffect()->parent_clip;

    for (int i=0;i<open_effects_.size();i++) {
      EffectUI* effect_ui = open_effects_.at(i);

      if (effect_ui->GetEffect()->parent_clip == first_clip) {
        NodeUI* node_ui = new NodeUI();

        effect_ui->SetNodeParent(node_ui);
        effect_ui->SetSelectable(false);

        node_ui->SetWidget(effect_ui);
        node_ui->AddToScene(&scene_);
        node_ui->setPos(effect_ui->GetEffect()->pos());

        nodes_.append(node_ui);

        // Get node edges
        QVector<NodeEdge*> edges = open_effects_.at(i)->GetEffect()->GetAllEdges();
        for (int j=0;j<edges.size();j++) {

          NodeEdge* edge = edges.at(j);

          if (!added_edges.contains(edge)) {
            NodeEdgeUI* edge_ui = new NodeEdgeUI(edge);
            scene_.addItem(edge_ui);
            edges_.append(edge_ui);

            added_edges.append(edge);
          }
        }
      }
    }

    ItemsChanged();
  }
}

void NodeEditor::ClearEvent()
{
  foreach (NodeUI* node, nodes_) {
    scene_.removeItem(node);
  }

  nodes_.clear();

  foreach (NodeEdgeUI* edge, edges_) {
    scene_.removeItem(edge);
  }

  edges_.clear();
}

void NodeEditor::ItemsChanged()
{
  foreach (NodeEdgeUI* edge, edges_) {
    edge->adjust();
  }

  foreach (NodeUI* node, nodes_) {
    node->Widget()->GetEffect()->SetPos(node->pos());
  }
}
