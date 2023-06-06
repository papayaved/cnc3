#-------------------------------------------------
#
# Project created by QtCreator 2019-01-11T11:24:45
#
#-------------------------------------------------

#unix:include ( /usr/local/qwt-6.3.0-svn/features/qwt.prf )
unix: include ( /usr/local/qwt-6.1.5/features/qwt.prf )
else: include ( C:\Qwt-6.2.0\features\qwt.prf )

unix: DEFINES += LINUX

#build_pass:CONFIG(debug, debug|release) {
#message(Debug bulid)
#}
#else:build_pass {
#message(Release build)
#DEFINES += QT_NO_DEBUG_OUTPUT
#}

Release: DEFINES += QT_NO_DEBUG_OUTPUT
unix: DEFINES += QT_NO_DEBUG_OUTPUT

QT       += core gui widgets
#requires(qtConfig(tableview))

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = icnc
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
QT += serialport
#QT += charts
CONFIG += qwt
QT += xml

SOURCES += \
    aux_items.cpp \
    center_widget.cpp \
    checkbox_delegate.cpp \
    cnc.cpp \
    cnc_backup.cpp \
    cnc_backup.cpp \
    cnc_com.cpp \
    cnc_com_thread.cpp \
    cnc_context.cpp \
    cnc_param.cpp \
    com_packet.cpp \
    combobox_delegate.cpp \
    contour_range.cpp \
    dialog_contour_properties.cpp \
    dialog_move_seg.cpp \
    dialog_segment_properties.cpp \
    form_center.cpp \
    form_help.cpp \
    form_mult.cpp \
    form_passes_stone.cpp \
    form_pult.cpp \
    form_settings.cpp \
    fpga.cpp \
    frect_t.cpp \
    generator_mode_list.cpp \
    gframe.cpp \
    gcode.cpp \
    layer_dialog.cpp \
    main.cpp \
    mainwindow.cpp \
    cnc_test.cpp \
    motor_record.cpp \
    form_edit.cpp \
    form_run.cpp \
    form_test.cpp \
    code_editor.cpp \
    cnc_program.cpp \
    form_home.cpp \
    my_app.cpp \
    my_lib.cpp \
    my_types.cpp \
    program_param.cpp \
    pult_widget.cpp \
    resize_dialog.cpp \
    rotate_dialog.cpp \
    slicing_widget.cpp \
    slots_widget.cpp \
    start_stop_elapsed_timer.cpp \
    run_widget.cpp \
    dxf.cpp \
    form_contour.cpp \
    new_cutline_dialog.cpp \
    form_passes.cpp \
    spinbox_delegate.cpp \
    contour_table_model.cpp \
    mode_table_model.cpp \
    gpoint.cpp \
    string_parser.cpp \
    contour_pair.cpp \
    contour_list.cpp \
    contour_pass.cpp \
    fpoint_t.cpp \
    cnc_types.cpp \
    double_ext.cpp \
    dxf_arc.cpp \
    dxf_line.cpp \
    dxf_entities.cpp \
    dxf_entity.cpp \
    dxf_circle.cpp \
    dxf_intersect.cpp \
    dxf_point.cpp \
    plot_view.cpp \
    qwt_plot_view.cpp \
    uv_sizing_widget.cpp \
    wire_cut.cpp \
    xml_ext.cpp

HEADERS += \
    amp.h \
    aux_items.h \
    center_widget.h \
    checkbox_delegate.h \
    cnc.h \
    cnc_backup.h \
    cnc_com.h \
    cnc_com_thread.h \
    cnc_context.h \
    cnc_param.h \
    com_packet.h \
    combobox_delegate.h \
    contour_range.h \
    contour_range.h \
    dialog_contour_properties.h \
    dialog_move_seg.h \
    dialog_segment_properties.h \
    form_center.h \
    form_help.h \
    form_mult.h \
    form_passes_stone.h \
    form_pult.h \
    form_settings.h \
    fpga.h \
    frect_t.h \
    generator_mode_list.h \
    gframe.h \
    gcode.h \
    layer_dialog.h \
    main.h \
    mainwindow.h \
    motor_record.h \
    form_test.h \
    form_run.h \
    form_edit.h \
    cnc_program.h \
    code_editor.h \
    form_home.h \
    my_app.h \
    my_lib.h \
    program_param.h \
    pult_widget.h \
    resize_dialog.h \
    rotate_dialog.h \
    slicing_widget.h \
    slots_widget.h \
    start_stop_elapsed_timer.h \
    run_widget.h \
    dxf.h \
    form_contour.h \
    debug.h \
    new_cutline_dialog.h \
    form_passes.h \
    spinbox_delegate.h \
    contour_table_model.h \
    mode_table_model.h \
    my_types.h \
    gpoint.h \
    string_parser.h \
    contour_pair.h \
    contour_list.h \
    contour_pass.h \
    fpoint_t.h \
    cnc_types.h \
    double_ext.h \
    dxf_entity.h \
    dxf_arc.h \
    dxf_entities.h \
    dxf_line.h \
    dxf_circle.h \
    dxf_intersect.h \
    dxf_point.h \
    plot_view.h \
    qwt_plot_view.h \
    uv_sizing_widget.h \
    uv_sizing_widget.h \
    wire_cut.h \
    xml_ext.h

FORMS += \
    mainwindow.ui \
    formtest.ui

TRANSLATIONS = icnc_ru.ts

RC_ICONS = icnc.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icnc_ru.qm
