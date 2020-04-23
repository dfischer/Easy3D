#include "widget_drawable_points.h"

#include <QColorDialog>

#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/texture_manager.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"
#include "main_window.h"

#include "ui_widget_drawable_points.h"


using namespace easy3d;


WidgetPointsDrawable::WidgetPointsDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui(new Ui::WidgetPointsDrawable) {
    ui->setupUi(this);

    if (colormaps_.empty())
        ui->comboBoxScalarFieldStyle->addItem("not available");
    else {
        for (const auto &colormap : colormaps_)
            ui->comboBoxScalarFieldStyle->addItem(QIcon(QString::fromStdString(colormap.file)),
                                                  QString::fromStdString("  " + colormap.name));
    }
}


void WidgetPointsDrawable::connectAll() {
    // which drawable
    connect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    connect(ui->doubleSpinBoxPointSize, SIGNAL(valueChanged(double)), this, SLOT(setPointSize(double)));

    // imposter
    connect(ui->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setImposterStyle(const QString &)));

    // lighting
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setLighting(const QString &)));

    // color scheme
    connect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    connect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    connect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // highlight
    connect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    connect(ui->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    connect(ui->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // scalar field
    connect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setScalarFieldStyle(int)));
    connect(ui->checkBoxScalarFieldClamp, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldClamp(bool)));
    connect(ui->doubleSpinBoxScalarFieldClampLower, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampLower(double)));
    connect(ui->doubleSpinBoxScalarFieldClampUpper, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampUpper(double)));

    // vector field
    connect(ui->comboBoxVectorField, SIGNAL(currentIndexChanged(const QString&)), this,
            SLOT(setVectorField(const QString&)));
    connect(ui->doubleSpinBoxVectorFieldScale, SIGNAL(valueChanged(double)), this, SLOT(setVectorFieldScale(double)));
}


void WidgetPointsDrawable::disconnectAll() {
    // which drawable
    disconnect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setActiveDrawable(const QString &)));

    // visible
    disconnect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    disconnect(ui->doubleSpinBoxPointSize, SIGNAL(valueChanged(double)), this, SLOT(setPointSize(double)));

    // imposter
    disconnect(ui->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setImposterStyle(const QString &)));

    // lighting
    disconnect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setLighting(const QString &)));

    // color scheme
    disconnect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    disconnect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    disconnect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // highlight
    disconnect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    disconnect(ui->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    disconnect(ui->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // scalar field
    disconnect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setScalarFieldStyle(int)));
    disconnect(ui->checkBoxScalarFieldClamp, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldClamp(bool)));
    disconnect(ui->doubleSpinBoxScalarFieldClampLower, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampLower(double)));
    disconnect(ui->doubleSpinBoxScalarFieldClampUpper, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampUpper(double)));

    // vector field
    disconnect(ui->comboBoxVectorField, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setVectorField(const QString&)));
    disconnect(ui->doubleSpinBoxVectorFieldScale, SIGNAL(valueChanged(double)), this, SLOT(setVectorFieldScale(double)));
}


WidgetPointsDrawable::~WidgetPointsDrawable() {
    delete ui;
}


std::vector<QString> WidgetPointsDrawable::colorSchemes(const easy3d::Model *model) {
    std::vector<QString> schemes;
    schemes.push_back("uniform color");

    auto cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        // color schemes from color properties and texture
        for (const auto &name : cloud->vertex_properties()) {
            if (name.find("v:color") != std::string::npos || name.find("v:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }

        // color schemes from scalar fields
        // scalar fields defined on vertices
        for (const auto &name : cloud->vertex_properties()) {
            if (cloud->get_vertex_property<float>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (cloud->get_vertex_property<double>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (cloud->get_vertex_property<unsigned int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (cloud->get_vertex_property<int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
        }
    }

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        // color schemes from color properties and texture
        for (const auto &name : mesh->vertex_properties()) {
            if (name.find("v:color") != std::string::npos || name.find("v:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }

        // color schemes from scalar fields
        // scalar fields defined on vertices
        for (const auto &name : mesh->vertex_properties()) {
            if (mesh->get_vertex_property<float>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<double>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<unsigned int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
        }
    }

    auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
    if (graph) {
        // color schemes from color properties and texture
        for (const auto &name : graph->vertex_properties()) {
            if (name.find("v:color") != std::string::npos || name.find("v:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }

        // color schemes from scalar fields
        // scalar fields defined on vertices
        for (const auto &name : graph->vertex_properties()) {
            if (graph->get_vertex_property<float>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (graph->get_vertex_property<double>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (graph->get_vertex_property<unsigned int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (graph->get_vertex_property<int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
        }
    }

    return schemes;
}


std::vector<QString> WidgetPointsDrawable::vectorFields(const easy3d::Model *model) {
    std::vector<QString> fields;

    auto cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        // vector fields defined on vertices
        for (const auto &name : cloud->vertex_properties()) {
            if (cloud->get_vertex_property<vec3>(name)) {
                if (name != "v:color" && name != "v:point")
                    fields.push_back(QString::fromStdString(name));
            }
        }
    }

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        // vector fields defined on vertices
        for (const auto &name : mesh->vertex_properties()) {
            if (mesh->get_vertex_property<vec3>(name)) {
                if (name != "v:color" && name != "v:point")
                    fields.push_back(QString::fromStdString(name));
            }
        }
    }

    auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
    if (graph) {
        // vector fields defined on vertices
        for (const auto &name : graph->vertex_properties()) {
            if (graph->get_vertex_property<vec3>(name)) {
                if (name != "v:color" && name != "v:point")
                    fields.push_back(QString::fromStdString(name));
            }
        }
    }

    // if no vector fields found, add a "not available" item
    if (fields.empty())
        fields.push_back("not available");
    else   // add one allowing to disable vector fields
        fields.insert(fields.begin(), "disabled");

    return fields;
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetPointsDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model || !model->is_visible() || model->points_drawables().empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

    auto d = dynamic_cast<PointsDrawable*>(drawable());
    auto &state = states_[d];
    auto &scheme = d->color_scheme();

    ui->comboBoxDrawables->clear();
    const auto &drawables = model->points_drawables();
    for (auto d : drawables)
        ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));
    ui->comboBoxDrawables->setCurrentText(QString::fromStdString(d->name()));

    // visible
    ui->checkBoxVisible->setChecked(d->is_visible());

    // thickness
    ui->doubleSpinBoxPointSize->setValue(d->point_size());

    // imposter
    switch (d->impostor_type()) {
        case PointsDrawable::PLAIN:
            ui->comboBoxImposterStyle->setCurrentText("plain");
            break;
        case PointsDrawable::SPHERE:
            ui->comboBoxImposterStyle->setCurrentText("sphere");
            break;
        case PointsDrawable::SURFEL:
            ui->comboBoxImposterStyle->setCurrentText("surfel");
            break;
    }

    {   // lighting
        if (d->lighting()) {
            if (d->lighting_two_sides())
                ui->comboBoxLightingOptions->setCurrentText("front and back");
            else
                ui->comboBoxLightingOptions->setCurrentText("front only");
        } else
            ui->comboBoxLightingOptions->setCurrentText("disabled");
    }

    {   // color scheme
        ui->comboBoxColorScheme->clear();
        const std::vector<QString> &schemes = colorSchemes(viewer_->currentModel());
        for (const auto &scheme : schemes)
            ui->comboBoxColorScheme->addItem(scheme);

        for (const auto& name : schemes) {
            if (name.contains(QString::fromStdString(scheme.name))) {
                ui->comboBoxColorScheme->setCurrentText(name);
                break;
            }
        }

        // default color
        vec3 c = d->default_color();
        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));

        // back side color
        ui->checkBoxBackColor->setChecked(d->distinct_back_color());
        c = d->back_color();
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));
    }

    {   // highlight
        bool highlight = d->highlight();
        ui->checkBoxHighlight->setChecked(highlight);

        const auto &range = d->highlight_range();
        ui->spinBoxHighlightMin->setValue(range.first);
        ui->spinBoxHighlightMax->setValue(range.second);
    }

    {   // scalar field
        ui->comboBoxScalarFieldStyle->setCurrentIndex(state.scalar_style);
        ui->checkBoxScalarFieldClamp->setChecked(scheme.clamp_value);
        ui->doubleSpinBoxScalarFieldClampLower->setValue(scheme.dummy_lower * 100);
        ui->doubleSpinBoxScalarFieldClampUpper->setValue(scheme.dummy_upper * 100);
    }

    {   // vector field
        ui->comboBoxVectorField->clear();
        const std::vector<QString> &fields = vectorFields(viewer_->currentModel());
        for (auto name : fields)
            ui->comboBoxVectorField->addItem(name);

        ui->comboBoxVectorField->setCurrentText(state.vector_field);
        ui->doubleSpinBoxVectorFieldScale->setValue(state.vector_field_scale);
    }

    disableUnavailableOptions();

    connectAll();

    state.initialized = true;
}


Drawable *WidgetPointsDrawable::drawable() {
    auto model = viewer_->currentModel();
    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->get_points_drawable(pos->second);
    else {
        const auto &drawables = model->points_drawables();
        if (drawables.empty())
            return nullptr;
        else {
            active_drawable_[model] = drawables[0]->name();
            return drawables[0];
        }
    }
}


void WidgetPointsDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    const std::string &name = text.toStdString();

    if (active_drawable_.find(model) != active_drawable_.end()) {
        if (active_drawable_[model] == name)
            return; // already active
    }

    if (model->get_points_drawable(name)) {
        active_drawable_[model] = name;
    } else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto &drawables = model->points_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no points drawable defined on model: " << model->name();
        else
            active_drawable_[model] = drawables[0]->name();
    }

    updatePanel();
}


void WidgetPointsDrawable::setPointSize(double s) {
    auto d = dynamic_cast<PointsDrawable*>(drawable());
    d->set_point_size(s);
    viewer_->update();
}


void WidgetPointsDrawable::setImposterStyle(const QString &style) {
    auto d = dynamic_cast<PointsDrawable*>(drawable());
    if (style == "plain")
        d->set_impostor_type(PointsDrawable::PLAIN);
    else if (style == "sphere")
        d->set_impostor_type(PointsDrawable::SPHERE);
    else if (style == "surfel") {
        if (d->normal_buffer() == 0) { // surfel requires point normals
            SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
            if (mesh) {
                auto normals = mesh->get_vertex_property<vec3>("v:normal");
                if (!normals) {
                    mesh->update_vertex_normals();
                    normals = mesh->get_vertex_property<vec3>("v:normal");
                }
                viewer_->makeCurrent();
                d->update_normal_buffer(normals.vector());
                viewer_->doneCurrent();
            } else if (dynamic_cast<PointCloud *>(viewer_->currentModel())) {
                if (d->normal_buffer() == 0) { // surfel requires point normals
                    PointCloud *cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
                    auto normals = cloud->get_vertex_property<vec3>("v:normal");
                    if (normals) {
                        viewer_->makeCurrent();
                        d->update_normal_buffer(normals.vector());
                        viewer_->doneCurrent();
                    }
                }
            }
        }

        if (d->normal_buffer())
            d->set_impostor_type(PointsDrawable::SURFEL);
        else
            LOG(WARNING) << "point imposter SURFEL requires normal information";
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetPointsDrawable::setColorScheme(const QString &text) {
    auto d = drawable();
    auto& scheme = d->color_scheme();
    scheme.clamp_value = ui->checkBoxScalarFieldClamp->isChecked();
    scheme.dummy_lower = ui->doubleSpinBoxScalarFieldClampLower->value() / 100.0;
    scheme.dummy_upper = ui->doubleSpinBoxScalarFieldClampUpper->value() / 100.0;
    states_[d].scalar_style = ui->comboBoxScalarFieldStyle->currentIndex();

    WidgetDrawable::setColorScheme(text);
}


void WidgetPointsDrawable::setDefaultColor() {
    auto d = drawable();
    const vec3 &c = d->default_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        d->set_default_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(color);
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }
}


void WidgetPointsDrawable::setBackColor() {
    auto d = drawable();
    const vec3 &c = d->back_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        d->set_back_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonBackColor->size());
        pixmap.fill(color);
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));
    }
}


void WidgetPointsDrawable::setVectorField(const QString &text) {
    auto model = viewer_->currentModel();

    auto drawa = drawable();
    if (text == "disabled") {
        const auto &drawables = model->lines_drawables();
        for (auto d : drawables) {
            if (d->name().find("vector - v") != std::string::npos)
                d->set_visible(false);
        }
        states_[drawa].vector_field = "disabled";
    } else {
        const std::string &name = text.toStdString();
        updateVectorFieldBuffer(model, name);

        auto d = model->get_lines_drawable("vector - " + name);
        d->set_visible(true);

        states_[drawa].vector_field = QString::fromStdString(name);
    }

    main_window_->updateRenderingPanel();
    viewer_->update();
}


void WidgetPointsDrawable::updateVectorFieldBuffer(Model *model, const std::string &name) {
    if (name == "v:normal") {
        if (dynamic_cast<SurfaceMesh *>(model)) {
            auto mesh = dynamic_cast<SurfaceMesh *>(model);
            auto normals = mesh->get_vertex_property<vec3>(name);
            if (!normals)
                mesh->update_vertex_normals();
        }
    }

    // a vector field is visualized as a LinesDrawable whose name is the same as the vector field
    auto drawable = model->get_lines_drawable("vector - v:normal");
    if (!drawable) {
        drawable = model->add_lines_drawable("vector - v:normal");
        drawable->set_update_func([this](Model *m, Drawable *d) -> void {
            const std::string& name = "v:normal";

            std::vector<vec3> vertices;
            if (dynamic_cast<SurfaceMesh *>(m)) {
                auto mesh = dynamic_cast<SurfaceMesh *>(m);
                auto prop = mesh->get_vertex_property<vec3>(name);
                if (!prop && name != "disabled") {
                    LOG(ERROR) << "vector field '" << name << "' doesn't exist";
                    return;
                }
                auto points = mesh->get_vertex_property<vec3>("v:point");

                // use a limited number of edge to compute the length of the vectors.
                float avg_edge_length = 0.0f;
                const int num = std::min(static_cast<unsigned int>(500), mesh->n_edges());
                for (unsigned int i = 0; i < num; ++i) {
                    SurfaceMesh::Edge edge(i);
                    auto vs = mesh->vertex(edge, 0);
                    auto vt = mesh->vertex(edge, 1);
                    avg_edge_length += distance(points[vs], points[vt]);
                }
                avg_edge_length /= num;

                vertices.resize(mesh->n_vertices() * 2, vec3(0.0f, 0.0f, 0.0f));
                int idx = 0;
                float scale = ui->doubleSpinBoxVectorFieldScale->value();
                for (auto v: mesh->vertices()) {
                    vertices[idx] = points[v];
                    vertices[idx + 1] = vertices[idx] + prop[v] * avg_edge_length * scale;
                    idx += 2;
                }
            }
            else if (dynamic_cast<PointCloud *>(m)) {
                auto cloud = dynamic_cast<PointCloud *>(m);

                auto prop = cloud->get_vertex_property<vec3>(name);
                if (!prop && name != "disabled") {
                    LOG(ERROR) << "vector field '" << name << "' doesn't exist";
                    return;
                }

                auto points = cloud->get_vertex_property<vec3>("v:point");

                // use the 1% of the bbox diagonal as the vector length.
                float avg_edge_length = cloud->bounding_box().diagonal() * 0.01f;

                vertices.resize(cloud->n_vertices() * 2, vec3(0.0f, 0.0f, 0.0f));
                int idx = 0;
                float scale = ui->doubleSpinBoxVectorFieldScale->value();
                for (auto v: cloud->vertices()) {
                    vertices[idx] = points[v];
                    vertices[idx + 1] = vertices[idx] + prop[v] * avg_edge_length * scale;
                    idx += 2;
                }
            }
            d->update_vertex_buffer(vertices);
        });
    }
}


void WidgetPointsDrawable::disableUnavailableOptions() {
    auto d = drawable();

    bool visible = ui->checkBoxVisible->isChecked();
    ui->labelPointSize->setEnabled(visible);
    ui->doubleSpinBoxPointSize->setEnabled(visible);
    ui->labelImposterStyle->setEnabled(visible);
    ui->comboBoxImposterStyle->setEnabled(visible);
    ui->labelLighting->setEnabled(visible);
    ui->comboBoxLightingOptions->setEnabled(visible);
    ui->labelColorScheme->setEnabled(visible);
    ui->comboBoxColorScheme->setEnabled(visible);

    bool can_modify_default_color = visible && (ui->comboBoxColorScheme->currentText() == "uniform color");
    ui->labelDefaultColor->setEnabled(can_modify_default_color);
    ui->toolButtonDefaultColor->setEnabled(can_modify_default_color);

    const auto &lighting_option = ui->comboBoxLightingOptions->currentText();
    bool can_modify_back_color = visible && lighting_option == "front and back" && ui->comboBoxImposterStyle->currentText() != "sphere";
    ui->labelBackColor->setEnabled(can_modify_back_color);
    ui->checkBoxBackColor->setEnabled(can_modify_back_color);
    ui->toolButtonBackColor->setEnabled(can_modify_back_color && d->distinct_back_color());

    bool can_modify_highlight = visible;
    ui->labelHighlight->setEnabled(can_modify_highlight);
    ui->checkBoxHighlight->setEnabled(can_modify_highlight);
    bool can_modify_highlight_range = can_modify_highlight && ui->checkBoxHighlight->isChecked();
    ui->spinBoxHighlightMin->setEnabled(can_modify_highlight_range);
    ui->spinBoxHighlightMax->setEnabled(can_modify_highlight_range);

    // scalar field
    bool can_show_scalar = visible && ui->comboBoxColorScheme->currentText().contains(scalar_prefix_);
    ui->labelScalarFieldStyle->setEnabled(can_show_scalar);
    ui->comboBoxScalarFieldStyle->setEnabled(can_show_scalar);
    ui->labelScalarFieldClamp->setEnabled(can_show_scalar);
    ui->checkBoxScalarFieldClamp->setEnabled(can_show_scalar);
    ui->doubleSpinBoxScalarFieldClampLower->setEnabled(can_show_scalar && ui->checkBoxScalarFieldClamp->isChecked());
    ui->doubleSpinBoxScalarFieldClampUpper->setEnabled(can_show_scalar && ui->checkBoxScalarFieldClamp->isChecked());

    // vector field
    bool can_show_vector = visible && ui->comboBoxVectorField->currentText() != "not available";
    ui->labelVectorField->setEnabled(can_show_vector);
    ui->comboBoxVectorField->setEnabled(can_show_vector);
    bool can_modify_vector_style = can_show_vector && ui->comboBoxVectorField->currentText() != "disabled";
    ui->labelVectorFieldScale->setEnabled(can_modify_vector_style);
    ui->doubleSpinBoxVectorFieldScale->setEnabled(can_modify_vector_style);

    update();
    qApp->processEvents();
}