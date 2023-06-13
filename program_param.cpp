#include "program_param.h"
#include <QSettings>
#include <QApplication>
#include <QDebug>
#include <string>

InterfaceLanguage ProgramParam::lang = InterfaceLanguage::ENGLISH;
int ProgramParam::fontSize = 10;
bool ProgramParam::swapXY = false;
bool ProgramParam::reverseX = false;
bool ProgramParam::reverseY = false;
bool ProgramParam::showXY = false;

ProgramParam::ProgramParam() {
//    m_SettingsFile = QApplication::applicationDirPath() + "/settings.ini";
//    qDebug() << m_SettingsFile;
    setDefaultGenModeList();
    loadSettings();
//    cnc.bindCncParam(&cncParam);
}

ProgramParam::~ProgramParam() {
    saveSettings();
}

QString ProgramParam::loadString(QSettings& settings, const QString& key, const QString& defaultValue, bool& OK) {
    QString res;

    if (settings.contains(key))
        res = settings.value(key, defaultValue).toString();
    else {
        res = defaultValue;
        settings.setValue(key, defaultValue);
        OK = false;
    }

    return res;
}

bool ProgramParam::loadBoolean(QSettings& settings, const QString& key, const bool defaultValue, bool& OK) {
    bool res;

    if (settings.contains(key))
        res = settings.value(key, defaultValue).toBool();
    else {
        res = defaultValue;
        settings.setValue(key, defaultValue);
        OK = false;
    }

    return res;
}

unsigned ProgramParam::loadUInt(QSettings& settings, const QString& key, const unsigned minValue, const unsigned maxValue, const unsigned defaultValue, bool& OK) {
    unsigned res;
    bool l_OK = settings.contains(key);

    if (l_OK)
        res = settings.value(key, defaultValue).toUInt();

    if (!l_OK || res < minValue || res > maxValue) {
        res = defaultValue;
        settings.setValue(key, defaultValue);
        OK = false;
    }

    return res;
}

double ProgramParam::loadDouble(QSettings& settings, const QString& key, const double minValue, const double maxValue, const double defaultValue, bool& OK) {
    double res;
    bool l_OK = settings.contains(key);

    if (l_OK)
        res = settings.value(key, defaultValue).toDouble();

    if (!l_OK || res < minValue || res > maxValue) {
        res = defaultValue;
        settings.setValue(key, defaultValue);
        OK = false;
    }

    return res;
}

void ProgramParam::loadSettings() {
    bool OK = true;
    QSettings settings(org, app);
    dxfDir = loadString(settings,   "dxfDir",   QDir::homePath() + dxfDirDefault,   OK);
    gcodeDir = loadString(settings, "gcodeDir", QDir::homePath() + gcodeDirDefault, OK);
    projDir = loadString(settings,  "projDir",  QDir::homePath() + projDirDefault,  OK);

    if (dxfDir == "")   dxfDir      = QDir::homePath() + dxfDirDefault;
    if (gcodeDir == "") gcodeDir    = QDir::homePath() + gcodeDirDefault;
    if (projDir == "")  projDir     = QDir::homePath() + projDirDefault;

    dxfFileName = loadString(settings, "dxfFileName", "", OK);
    gcodeFileName = loadString(settings, "gcodeFileName", "", OK);
    projFileName = loadString(settings, "projFileName", "", OK);
    parFileName = loadString(settings, "adjFileName", "", OK);
    modesFileName = loadString(settings, "modesFileName", "", OK);
}

void ProgramParam::saveSettings() {
//    QSettings settings(m_SettingsFile, QSettings::NativeFormat);
    QSettings settings(org, app);
    settings.setValue("dxfDir", dxfDir);
    settings.setValue("gcodeDir", gcodeDir);
    settings.setValue("projDir", projDir);
    settings.setValue("dxfFileName", dxfFileName);
    settings.setValue("gcodeFileName", gcodeFileName);
    settings.setValue("projFileName", projFileName);
    settings.setValue("adjFileName", parFileName);
    settings.setValue("modesFileName", modesFileName);
    //    settings.sync();
}

void ProgramParam::saveCncContext() {
    QSettings settings(org, app);
    QByteArray bytes = cncContext.getByteArray();
    settings.setValue("cnc_context_t",  bytes);
}

void ProgramParam::clearCncContext() {
    cncContext.setValid(false);
    saveCncContext();
}

void ProgramParam::loadCncContext() {
    QSettings settings(org, app);

    if (!settings.contains("cnc_context_t"))
        cncContext.setDefault();
    else {
        QByteArray bytes = settings.value("cnc_context_t", QByteArray()).toByteArray();
        cncContext.set(bytes);
    }
}

// save G-code as contex
void ProgramParam::saveGcode() {
    QSettings settings(org, app);
    clearCncContext();
    settings.setValue("gcodeCnc", gcodeCnc);
//    saveInputLevel(CncParam::inputLevel);
}

// Load G-code from local settings file
QString ProgramParam::loadGcode() {
    QSettings settings(org, app);
    bool OK = true;

    loadInputLevel(OK);

    if (OK)
        return settings.value("gcodeCnc", QString()).toString();

    return QString();
}

void ProgramParam::setDefaultGenModeList() {
    genModesList.clear();
    genModesList.push_back(GeneratorMode(7, 4, 28, 8, false));
    genModesList.push_back(GeneratorMode(3, 3, 8, 8, false));
    genModesList.push_back(GeneratorMode(1, 2, 2, 8, true));
}

void ProgramParam::setDefaultCutParam() {
    cutParam = cut_t();
}

// Parse a text file into the binary structure
GCode ProgramParam::compile(const QString& txt) {
    using namespace std;

    GCode gcode;
    int err = gcode.parse(txt.toStdString());

    if (err == 0) {
        string new_txt = gcode.toText();
        qDebug() << new_txt.c_str();
    }
    else
        gcode.clear();

    return gcode;
}

void ProgramParam::loadBackup() {
    loadCncContext();

    QString txt = loadGcode();
    gcode = compile(txt);

    if (gcode.empty())
        clearCncContext();
    else {
        gcodeSettings = gcode.getSettings();
        gcodeText = gcode.toText().c_str();
    }
}

void ProgramParam::saveInterfaceLanguage(InterfaceLanguage lang) {
    QSettings settings(org, app);
    settings.setValue("language", static_cast<uint8_t>(lang));
    settings.sync();
}

InterfaceLanguage ProgramParam::loadInterfaceLanguage() {
    bool OK = true;
    QSettings settings(org, app);
    return lang = static_cast<InterfaceLanguage>(
                loadUInt(settings, "language", 0, static_cast<unsigned>(InterfaceLanguage::END) - 1, static_cast<unsigned>(InterfaceLanguage::ENGLISH), OK)
                );
}

QString ProgramParam::helpSubDir() {
#if defined(STONE)
    switch (lang) {
    case InterfaceLanguage::ENGLISH: return "stone/en";
    case InterfaceLanguage::RUSSIAN: return "stone/ru";
    default: return "stone/en";
    }
#else
    switch (lang) {
    case InterfaceLanguage::ENGLISH: return "metal/en";
    case InterfaceLanguage::RUSSIAN: return "metal/ru";
    default: return "metal/en";
    }
#endif
}

void ProgramParam::saveFontSize(int fontSize) {
    QSettings settings(org, app);
    ProgramParam::fontSize = fontSize;
    settings.setValue("fontSize", static_cast<uint8_t>(fontSize));
    settings.sync();
}

int ProgramParam::loadFontSize() {
    bool OK = true;
    QSettings settings(org, app);
    return fontSize = loadUInt(settings, "fontSize", 8, 32, 10, OK);
}

void ProgramParam::saveInputLevel(uint16_t value) {
    QSettings settings(org, app);
    CncParam::inputLevel = value;
    settings.setValue("inputLevel", CncParam::inputLevel);
    settings.sync();
}

uint16_t ProgramParam::loadInputLevel(bool& OK) {
    QSettings settings(org, app);    
    CncParam::inputLevel = static_cast<uint16_t>( loadUInt(settings, "inputLevel", 0, UINT16_MAX, CncParam::DEFAULT_INPUT_LEVEL, OK) );
    return CncParam::inputLevel;
}

void ProgramParam::saveStepDir(bool sdEna) {
    QSettings settings(org, app);
    CncParam::sdEnable = sdEna;
    settings.setValue("sdEnable", CncParam::sdEnable);
    settings.sync();
}

bool ProgramParam::loadStepDir(bool &OK) {
    QSettings settings(org, app);
    return CncParam::sdEnable = loadBoolean(settings, "sdEnable", CncParam::DEFAULT_SD_ENA, OK);
}

void ProgramParam::saveMotorDir(bool revX, bool revY, bool revU, bool revV, bool swapXY, bool swapUV, bool reverseEncX, bool reverseEncY) {
    QSettings settings(org, app);
    CncParam::reverseX = revX;
    CncParam::reverseY = revY;
    CncParam::reverseU = revU;
    CncParam::reverseV = revV;
    CncParam::swapXY = swapXY;
    CncParam::swapUV = swapUV;
    CncParam::reverseEncX = reverseEncX;
    CncParam::reverseEncY = reverseEncY;

    settings.setValue("motorReverseX", CncParam::reverseX);
    settings.setValue("motorReverseY", CncParam::reverseY);
    settings.setValue("motorReverseU", CncParam::reverseU);
    settings.setValue("motorReverseV", CncParam::reverseV);
    settings.setValue("motorSwapXY", CncParam::swapXY);
    settings.setValue("motorSwapUV", CncParam::swapUV);
    settings.setValue("encReverseX", CncParam::reverseEncX);
    settings.setValue("encReverseY", CncParam::reverseEncY);
    settings.sync();
}

void ProgramParam::saveFeedbackParam(bool fb_ena, double low_thld[2], double high_thld[2], double rb_to, unsigned rb_attempts, double rb_len, double rb_speed) {
    QSettings settings(org, app);
    CncParam::fb_ena = fb_ena;

    CncParam::low_thld[0] = low_thld[0];
    CncParam::low_thld[1] = low_thld[1];
    CncParam::high_thld[0] = high_thld[0];
    CncParam::high_thld[1] = high_thld[1];

    CncParam::rb_to = rb_to;
    CncParam::rb_attempts = rb_attempts;
    CncParam::rb_len = rb_len;
    CncParam::rb_speed = rb_speed;

    settings.setValue("feedbackEnable", CncParam::fb_ena);

    settings.setValue("lowThreshold", CncParam::low_thld[0]);
    settings.setValue("highThreshold", CncParam::high_thld[0]);
    settings.setValue("lowThresholdLow", CncParam::low_thld[1]);
    settings.setValue("highThresholdLow", CncParam::high_thld[1]);

    settings.setValue("rollbackTimeout", CncParam::rb_to);
    settings.setValue("rollbackAttempts", CncParam::rb_attempts);
    settings.setValue("rollbackLength", CncParam::rb_len);
    settings.setValue("rollbackSpeed", CncParam::rb_speed);

    settings.sync();
}

void ProgramParam::saveFeedbackAcceleration(double fb_acc, double fb_dec) {
    QSettings settings(org, app);
    CncParam::fb_acc = fb_acc;
    CncParam::fb_dec = fb_dec;

    settings.setValue("fb_acc", CncParam::fb_acc);
    settings.setValue("fb_dec", CncParam::fb_dec);
    settings.sync();
}

void ProgramParam::saveAcceleration(bool ena, double acc, double dec) {
    QSettings settings(org, app);
    CncParam::acc_ena = ena;
    CncParam::acc = acc;
    CncParam::dec = dec;

    settings.setValue("acc_ena", CncParam::acc_ena);
    settings.setValue("acc", CncParam::acc);
    settings.setValue("dec", CncParam::dec);
    settings.sync();
}

void ProgramParam::saveStep(double step, double scaleX, double scaleY, double scaleU, double scaleV, double scaleEncX, double scaleEncY, bool encXY) {
    QSettings settings(org, app);
    CncParam::step = step;
    CncParam::scaleX = scaleX;
    CncParam::scaleY = scaleY;
    CncParam::scaleU = scaleU;
    CncParam::scaleV = scaleV;
    CncParam::scaleEncX = scaleEncX;
    CncParam::scaleEncY = scaleEncY;
    CncParam::encXY = encXY;

    settings.setValue("step", CncParam::step);
    settings.setValue("scaleX", CncParam::scaleX);
    settings.setValue("scaleY", CncParam::scaleY);
    settings.setValue("scaleU", CncParam::scaleU);
    settings.setValue("scaleV", CncParam::scaleV);
    settings.setValue("scaleEncX", CncParam::scaleEncX);
    settings.setValue("scaleEncY", CncParam::scaleEncY);
    settings.setValue("encXY", CncParam::encXY);

    settings.sync();
}

bool ProgramParam::loadMotorReverseX() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::reverseX = loadBoolean(settings, "motorReverseX", false, OK);
}

bool ProgramParam::loadMotorReverseY() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::reverseY = loadBoolean(settings, "motorReverseY", false, OK);
}

bool ProgramParam::loadMotorReverseU() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::reverseU = loadBoolean(settings, "motorReverseU", false, OK);
}

bool ProgramParam::loadMotorReverseV() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::reverseV = loadBoolean(settings, "motorReverseV", false, OK);
}

bool ProgramParam::loadMotorSwapXY() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::swapXY = loadBoolean(settings, "motorSwapXY", false, OK);
}

bool ProgramParam::loadMotorSwapUV() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::swapUV = loadBoolean(settings, "motorSwapUV", false, OK);
}

bool ProgramParam::loadEncReverseX() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::reverseEncX = loadBoolean(settings, "encReverseX", false, OK);
}

bool ProgramParam::loadEncReverseY() {
    bool OK = true;
    QSettings settings(org, app);
    return CncParam::reverseEncY = loadBoolean(settings, "encReverseY", false, OK);
}

bool ProgramParam::loadFeedbackParam(
        bool& fb_ena, double (&low_thld)[2], double (&high_thld)[2], double& rb_to, unsigned& rb_attempts, double& rb_len, double& rb_speed
){
    bool OK = true;
    QSettings settings(org, app);
    fb_ena = CncParam::fb_ena = loadBoolean(settings, "feedbackEnable", false, OK);

    int thld_max = static_cast<int>( round(cnc_adc_volt_t::maxVolt(0)) );

    low_thld[0] = CncParam::low_thld[0] = loadDouble(settings, "lowThreshold", 0, thld_max, 0, OK);
    high_thld[0] = CncParam::high_thld[0] = loadDouble(settings, "highThreshold", 0, thld_max, thld_max, OK);

    low_thld[1] = CncParam::low_thld[1] = loadDouble(settings, "lowThresholdLow", 0, thld_max, 0, OK);
    high_thld[1] = CncParam::high_thld[1] = loadDouble(settings, "highThresholdLow", 0, thld_max, thld_max, OK);

    if (low_thld > high_thld) {
        low_thld[0] = CncParam::low_thld[0] = 0;
        low_thld[1] = CncParam::low_thld[1] = 0;

        high_thld[0] = CncParam::high_thld[0] = thld_max;
        high_thld[1] = CncParam::high_thld[1] = thld_max;

        settings.setValue("lowThreshold", CncParam::low_thld[0]);
        settings.setValue("highThreshold", CncParam::high_thld[0]);

        settings.setValue("lowThresholdLow", CncParam::low_thld[1]);
        settings.setValue("highThresholdLow", CncParam::high_thld[1]);
    }

    rb_to = CncParam::rb_to = loadDouble(settings, "rollbackTimeout", 0, CncParam::DEFAULT_RB_TO * 10, CncParam::DEFAULT_RB_TO, OK);
    rb_attempts = CncParam::rb_attempts = loadUInt(settings, "rollbackAttempts", 1, CncParam::DEFAULT_RB_ATTEMPTS * 10, CncParam::DEFAULT_RB_ATTEMPTS, OK);
    rb_len = CncParam::rb_len = loadDouble(settings, "rollbackLength", 0, CncParam::DEFAULT_RB_LEN * 10, CncParam::DEFAULT_RB_LEN, OK);
    rb_speed = CncParam::rb_speed = loadDouble(settings, "rollbackSpeed", 0, CncParam::DEFAULT_RB_SPEED * 10, CncParam::DEFAULT_RB_SPEED, OK);

    return OK;
}

bool ProgramParam::loadFeedbackAcceleration(double &fb_acc, double &fb_dec) {
    bool OK = true;
    QSettings settings(org, app);
    fb_acc = CncParam::fb_acc = loadDouble(settings, "fb_acc", CncParam::DEFAULT_ACC * 0.1, CncParam::DEFAULT_ACC * 10, CncParam::DEFAULT_ACC, OK);
    fb_dec = CncParam::fb_dec = loadDouble(settings, "fb_dec", CncParam::DEFAULT_DEC * 0.1, CncParam::DEFAULT_DEC * 10, CncParam::DEFAULT_DEC, OK);
    return OK;
}

bool ProgramParam::loadAcceleration(bool& ena, double &acc, double &dec) {
    bool OK = true;
    QSettings settings(org, app);
    ena = CncParam::acc_ena = loadBoolean(settings, "acc_ena", CncParam::DEFAULT_ACC_ENA, OK);
    acc = CncParam::acc = loadDouble(settings, "acc", CncParam::DEFAULT_ACC * 0.1, CncParam::DEFAULT_ACC * 10, CncParam::DEFAULT_ACC, OK);
    dec = CncParam::dec = loadDouble(settings, "dec", CncParam::DEFAULT_DEC * 0.1, CncParam::DEFAULT_DEC * 10, CncParam::DEFAULT_DEC, OK);
    return OK;
}

bool ProgramParam::loadStep(double &step, double &scaleX, double &scaleY, double &scaleU, double &scaleV, double &scaleEncX, double &scaleEncY, bool &encXY) {
    bool OK = true;
    QSettings settings(org, app);

    step = CncParam::step = loadDouble(settings, "step", CncParam::STEP_MIN, CncParam::STEP_MAX, CncParam::DEFAULT_STEP, OK);
    scaleX = CncParam::scaleX = loadDouble(settings, "scaleX", CncParam::SCALE_MIN, CncParam::SCALE_MAX, CncParam::DEFAULT_SCALE_XY, OK);
    scaleY = CncParam::scaleY = loadDouble(settings, "scaleY", CncParam::SCALE_MIN, CncParam::SCALE_MAX, CncParam::DEFAULT_SCALE_XY, OK);
    scaleU = CncParam::scaleU = loadDouble(settings, "scaleU", CncParam::SCALE_MIN, CncParam::SCALE_MAX, CncParam::DEFAULT_SCALE_UV, OK);
    scaleV = CncParam::scaleV = loadDouble(settings, "scaleV", CncParam::SCALE_MIN, CncParam::SCALE_MAX, CncParam::DEFAULT_SCALE_UV, OK);
    scaleEncX = CncParam::scaleEncX = loadDouble(settings, "scaleEncX", CncParam::SCALE_ENC_MIN, CncParam::SCALE_ENC_MAX, CncParam::DEFAULT_SCALE_ENC_XY, OK);
    scaleEncY = CncParam::scaleEncY = loadDouble(settings, "scaleEncY", CncParam::SCALE_ENC_MIN, CncParam::SCALE_ENC_MAX, CncParam::DEFAULT_SCALE_ENC_XY, OK);
    encXY = CncParam::encXY = loadBoolean(settings, "encXY", CncParam::DEFAULT_ENC_XY, OK);

    return OK;
}

void ProgramParam::loadParam() {
    bool OK = true, fb_ena;
    unsigned rb_attempts;
    double low_thld[2], high_thld[2];
    double rb_to, rb_len, rb_speed, fb_acc, fb_dec;
    double step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY;
    bool encXY, acc_ena;
    double acc, dec;

    loadInterfaceLanguage();
    loadFontSize();

    loadSwapXY();
    loadReverseX();
    loadReverseY();
    loadShowXY();

    loadInputLevel(OK);

    loadMotorReverseX();
    loadMotorReverseY();
    loadMotorReverseU();
    loadMotorReverseV();
    loadMotorSwapXY();
    loadMotorSwapUV();

    loadEncReverseX();
    loadEncReverseY();

    OK &= loadFeedbackParam(fb_ena, low_thld, high_thld, rb_to, rb_attempts, rb_len, rb_speed);
    OK &= loadFeedbackAcceleration(fb_acc, fb_dec);
    OK &= loadStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);
    OK &= loadAcceleration(acc_ena, acc, dec);

    loadStepDir(OK);
}

void ProgramParam::loadDefaultParam() {
    lang = InterfaceLanguage::ENGLISH;
    swapXY = false;
    reverseX = reverseY = false;
    showXY = false;
    CncParam::reset();
}

//
void ProgramParam::saveSwapXY(bool value) {
    QSettings settings(org, app);
    swapXY = value;
    settings.setValue("swapXY", swapXY);
    settings.sync();
}

bool ProgramParam::loadSwapXY() {
    bool OK = true;
    QSettings settings(org, app);
    return swapXY = loadBoolean(settings, "swapXY", false, OK);
}

void ProgramParam::saveReverseX(bool value) {
    QSettings settings(org, app);
    reverseX = value;
    settings.setValue("reverseX", reverseX);
    settings.sync();
}

bool ProgramParam::loadReverseX() {
    bool OK = true;
    QSettings settings(org, app);
    return reverseX = loadBoolean(settings, "reverseX", false, OK);
}

void ProgramParam::saveReverseY(bool value) {
    QSettings settings(org, app);
    reverseY = value;
    settings.setValue("reverseY", reverseY);
    settings.sync();
}

bool ProgramParam::loadReverseY() {
    bool OK = true;
    QSettings settings(org, app);
    return reverseY = loadBoolean(settings, "reverseY", false, OK);
}

void ProgramParam::saveShowXY(bool value) {
    QSettings settings(org, app);
    showXY = value;
    settings.setValue("showXY", showXY);
    settings.sync();
}

bool ProgramParam::loadShowXY() {
    bool OK = true;
    QSettings settings(org, app);
    return showXY = loadBoolean(settings, "showXY", false, OK);
}

//enum BUTTON_T {START, REVERSE, CANCEL};
//enum CNC_STATES_T {UNCHECKED, IDLE, RUN, PAUSE, REVERSE, CANCEL, SHORT_REVERSE, ERROR};

void AppState::next(AppState::BUTTON btn) {
    switch (m_state) {
    case STATES::ST_NONE: case STATES::ST_IDLE:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_RUN;
        break;
    case STATES::ST_RUN:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_PAUSE;
        break;
    case STATES::ST_PAUSE:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_RUN;
        else if (btn == BUTTON::SIG_REVERSE)
            m_state = STATES::ST_REV;
        else if (btn == BUTTON::SIG_CANCEL)
            m_state = STATES::ST_CANCEL;
        break;
    case STATES::ST_REV:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_RUN;
        break;
    case STATES::ST_CANCEL:
        if (btn == BUTTON::SIG_REVERSE)
            m_state = STATES::ST_SHORT_REV;
        else if (btn == BUTTON::SIG_CANCEL)
            m_state = STATES::ST_NONE;
        break;
    case STATES::ST_SHORT_REV:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_CANCEL;
        break;
    case STATES::ST_ERROR:
        if (btn == BUTTON::SIG_CANCEL)
            m_state = STATES::ST_NONE;
        break;
    default:
        m_state = STATES::ST_ERROR;
        break;
    }
}

void ProgramParam::saveComb(const comb_t& comb) {
    QSettings settings(org, app);    
    m_comb = comb;

    settings.beginGroup("comb");
    settings.setValue("axis", (uint8_t)comb.axis);

    settings.setValue("roll_vel", comb.roll_vel);
    settings.setValue("roll_vel_idle", comb.roll_vel_idle);

    settings.setValue("slots_num", comb.slots_num);

    settings.setValue("pause_ena", comb.pause_ena);
    settings.setValue("pause", comb.pause);

    settings.setValue("wireD", comb.wireD);
    settings.setValue("depth", comb.depth);
    settings.setValue("step", comb.step);

    settings.setValue("speed", comb.speed);
    settings.setValue("speed_idle", comb.speed_idle);
    settings.endGroup();

    settings.sync();
}

bool ProgramParam::loadComb(comb_t& comb) {
    bool OK = true;
    QSettings settings(org, app);

    settings.beginGroup("comb");
    comb.axis           = (AXIS)loadUInt(settings, "axis", (uint)AXIS::AXIS_X, (uint)AXIS::AXIS_Y, (uint)AXIS::AXIS_X, OK);

    comb.roll_vel       = loadUInt(settings,    "roll_vel",      cnc_param::ROLL_MIN, cnc_param::ROLL_MAX, cnc_param::ROLL_MAX, OK);
    comb.roll_vel_idle  = loadUInt(settings,    "roll_vel_idle", cnc_param::ROLL_MIN, cnc_param::ROLL_MAX, cnc_param::ROLL_MIN, OK);

    comb.slots_num      = loadUInt(settings,    "slots_num", 1, 1000, 20, OK);

    comb.pause          = loadUInt(settings,    "pause", 0, 600, 30, OK);
    if (comb.pause == 0)
        comb.pause_ena  = false;
    else
        comb.pause_ena  = loadBoolean(settings,    "pause_ena", false, OK);

    comb.wireD          = loadDouble(settings,  "wireD", 0, 3, cnc_param::WIRE_DIAMETER, OK);
    comb.depth          = loadDouble(settings,  "depth", -1000, 1000, 40, OK);
    comb.step           = loadDouble(settings,  "step", -1000, 1000, -1, OK);
    comb.speed          = loadDouble(settings,  "speed", 0.1, cnc_param::SPEED_MAX, cnc_param::SPEED, OK);
    comb.speed_idle     = loadDouble(settings,  "speed_idle", 0.1, cnc_param::SPEED_MAX, cnc_param::SPEED_MAX, OK);
    settings.endGroup();

    if (OK)
        m_comb = comb;

    return OK;
}

void ProgramParam::saveSlices(const snake_t& snake) {
    QSettings settings(org, app);
    m_snake = snake;

    settings.beginGroup("slicing");
    settings.setValue("profile", (uint8_t)snake.profile);
    settings.setValue("axis", (uint8_t)snake.axis);

    settings.setValue("roll_vel", snake.roll_vel);
    settings.setValue("roll_vel_idle", snake.roll_vel_idle);

    settings.setValue("slices_num", snake.slices_num);
    settings.setValue("sections_num", snake.sections_num);

    settings.setValue("pause_ena", snake.pause_ena);
    settings.setValue("pause", snake.pause);

    settings.setValue("wireD", snake.wireD);
    settings.setValue("width", snake.width);
    settings.setValue("spacing", snake.spacing);
    settings.setValue("step", snake.step);

    settings.setValue("speed_avg", snake.speed_avg);
    settings.setValue("speed_idle", snake.speed_idle);
    settings.endGroup();

    settings.sync();
}

bool ProgramParam::loadSlices(snake_t& snake) {
    bool OK = true;
    QSettings settings(org, app);

    settings.beginGroup("slicing");
    snake.profile        = (SLICING_PROFILE)loadUInt(settings, "profile", (uint)SLICING_PROFILE::RECT, (uint)SLICING_PROFILE::ROUND, (uint)SLICING_PROFILE::RECT, OK);
    snake.axis           = (AXIS)loadUInt(settings, "axis", (uint)AXIS::AXIS_X, (uint)AXIS::AXIS_Y, (uint)AXIS::AXIS_X, OK);

    snake.roll_vel       = loadUInt(settings,    "roll_vel",      cnc_param::ROLL_MIN, cnc_param::ROLL_MAX, cnc_param::ROLL_MAX, OK);
    snake.roll_vel_idle  = loadUInt(settings,    "roll_vel_idle", cnc_param::ROLL_MIN, cnc_param::ROLL_MAX, cnc_param::ROLL_MIN, OK);

    snake.slices_num     = loadUInt(settings,    "slices_num", 1, 1000, 20, OK);
    snake.sections_num   = loadUInt(settings,    "sections_num", 1, 100, 20, OK);

    snake.pause          = loadUInt(settings,    "pause", 0, 600, 30, OK);
    if (snake.pause == 0)
        snake.pause_ena  = false;
    else
        snake.pause_ena  = loadBoolean(settings,    "pause_ena", false, OK);

    snake.wireD          = loadDouble(settings,  "wireD", 0, 3, cnc_param::WIRE_DIAMETER, OK);
    snake.width          = loadDouble(settings,  "width", -1000, 1000, 40, OK);
    snake.spacing        = loadDouble(settings,  "spacing", 1, 20, 2, OK);
    snake.step           = loadDouble(settings,  "step", -1000, 1000, -1, OK);

    snake.speed_avg      = loadDouble(settings,  "speed_avg", 0.1, cnc_param::SPEED_MAX, cnc_param::SPEED, OK);
    snake.speed_idle     = loadDouble(settings,  "speed_idle", 0.1, cnc_param::SPEED_MAX, cnc_param::SPEED_MAX, OK);
    settings.endGroup();

    if (OK)
        m_snake = snake;

    return OK;
}
