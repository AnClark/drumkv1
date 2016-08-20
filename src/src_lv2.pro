# drumkv1_lv2.pro
#
NAME = drumkv1

TARGET = $${NAME}_lv2
TEMPLATE = lib
CONFIG += shared plugin

include(src_lv2.pri)

HEADERS = \
	config.h \
	drumkv1.h \
	drumkv1_config.h \
	drumkv1_param.h \
	drumkv1_programs.h \
	drumkv1_controls.h \
	drumkv1_lv2.h

SOURCES = \
	drumkv1_lv2.cpp


unix {

	OBJECTS_DIR = .obj_lv2
	MOC_DIR     = .moc_lv2
	UI_DIR      = .ui_lv2

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	contains(PREFIX, $$system(echo $HOME)) {
		LV2DIR = $${PREFIX}/.lv2
	} else {
		isEmpty(LIBDIR) {
			LV2DIR = $${PREFIX}/lib/lv2
		} else {
			LV2DIR = $${LIBDIR}/lv2
		}
	}

	TARGET_LV2 = $${NAME}.lv2/$${NAME}

	INSTALLS += target

	target.path  = $${LV2DIR}/$${NAME}.lv2
	target.files = $${TARGET_LV2}.so \
		$${TARGET_LV2}.ttl \
		$${NAME}.lv2/manifest.ttl

	QMAKE_POST_LINK += $${QMAKE_COPY} -vp $(TARGET) $${TARGET_LV2}.so

	QMAKE_CLEAN += $${TARGET_LV2}.so

	LIBS += -L. -l$${NAME}
}

QT -= gui
QT += xml
