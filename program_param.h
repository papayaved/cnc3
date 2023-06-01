#ifndef PROGRAMPARAM_H
#define PROGRAMPARAM_H

#include <cstdint>
#include <QString>

#include "gcode.h"
#include "cnc.h"
#include "contour_list.h"
#include "cnc_types.h"

// TODO: remake it
// Class of buttons states in Run Window
class AppState {
public:
    // Buttons in Run Window
    enum class BUTTON {SIG_START, SIG_REVERSE, SIG_CANCEL, SIG_IMIT, SIG_ERROR};
    // States of Run Window
    enum class STATES {ST_NONE, ST_IDLE, ST_RUN, ST_PAUSE, ST_REV, ST_CANCEL, ST_SHORT_REV, ST_ERROR};
private:
    STATES m_state {STATES::ST_NONE};
    bool m_idle_run {false}; // Idle run
public:
    inline STATES state() const { return m_state; }
    inline bool isIdle() const { return m_state <= STATES::ST_IDLE; }
    inline bool isWork() const { return m_state > STATES::ST_IDLE && m_state < STATES::ST_ERROR; }
    inline bool isError() const { return m_state >= STATES::ST_ERROR; }

    inline void reset() { m_state = STATES::ST_NONE; }
    inline void setError() { m_state = STATES::ST_ERROR; }

    void next(AppState::BUTTON btn);

    // Work without restrictions
    void setIdleRun(bool value) { m_idle_run = value; }
    bool idleRun() const { return m_idle_run; }

    // Update buttons state
    inline void update(const CncContext& ctx) {
        if (ctx.isError())
            m_state = STATES::ST_ERROR;
        else if (ctx.isIdle())
            m_state = STATES::ST_NONE;
        else if (ctx.reverse())
            switch (ctx.cncState()) {
            case cnc_state_t::ST_READ:
            case cnc_state_t::ST_SEGMENT:
            case cnc_state_t::ST_WAIT:
            case cnc_state_t::ST_PAUSE:
            case cnc_state_t::ST_WAIT_BUTTON:
                m_state = STATES::ST_REV;
                break;

            case cnc_state_t::ST_STOP:
                m_state = STATES::ST_PAUSE;
                break;

            default:
                break;
            }
        else
            switch (ctx.cncState()) {
            case cnc_state_t::ST_READ:
            case cnc_state_t::ST_SEGMENT:
            case cnc_state_t::ST_WAIT:
            case cnc_state_t::ST_PAUSE:
            case cnc_state_t::ST_WAIT_BUTTON:
            case cnc_state_t::ST_END:
                m_state = STATES::ST_RUN;
                break;

            case cnc_state_t::ST_STOP:
                m_state = STATES::ST_PAUSE;
                break;

            default:
                break;
            }
    }

    inline void gotoPauseState() {
        m_state = STATES::ST_PAUSE;
    }
};

// Application language
enum class InterfaceLanguage : uint8_t { ENGLISH, RUSSIAN, END };

class ProgramParam {
    static constexpr const char* org = "Meatec";
    static constexpr const char* app = "CncPrototype3";

public:
    static InterfaceLanguage lang;
    static int fontSize;
    static bool swapXY, reverseX, reverseY, showXY; // swap and reverse axes on the screen

    QString gcodeText; // G-code for Text Editor
    QString gcodeCnc; // G-code for CNC

    QString dxfDir, gcodeDir, projDir; // current file directory

    const QString dxfDirDefault =   "/Work/DXF"; // "/home/cnc/Work/DXF"
    const QString gcodeDirDefault = "/Work/NC";
    const QString projDirDefault =  "/Work/Project";

    QString gcodeFileName, dxfFileName, modesFileName, parFileName, projFileName; // data files

    AppState appState;
    GCode gcode; // current parsed G-code
    GCodeSettings gcodeSettings; // extracted common parameters
    ContourList contours; // one pass contour from file or generated from templates

    ContourList workContours; // a work multi-pass contour for cutting
    std::vector<std::pair<size_t, size_t>> mapGcodeToContours; // relation between G-code frame (string) number and contour segment.

    Cnc cnc; // CNC programming interface (API)
    bool cncConnected;

    CncContext cncContext; // It's a variable for the work context that is received from CNC

    cut_t cutParam;
    std::deque<GeneratorMode> genModesList; // list of generator modes

    snake_t m_snake;
    comb_t m_comb;

    QMutex mutexParamLocker;

    ProgramParam();
    ~ProgramParam();

private:
    static QString loadString(QSettings& settings, const QString& key, const QString& defaultValue, bool& OK);
    static bool loadBoolean(QSettings& settings, const QString& key, const bool defaultValue, bool& OK);
    static unsigned loadUInt(QSettings& settings, const QString& key, const unsigned minValue, const unsigned maxValue, const unsigned defaultValue, bool& OK);
    static double loadDouble(QSettings& settings, const QString& key, const double minValue, const double maxValue, const double defaultValue, bool& OK);

public:
    void loadSettings();
    void saveSettings();

    void setCncContext(const cnc_context_t& ctx) { cncContext.set(ctx); }
    void saveCncContext();
    void loadCncContext();
    void clearCncContext();

    void saveGcode();
    QString loadGcode();
    const QString& getGcode() const { return gcodeCnc; }

    void setDefaultGenModeList();
    void setDefaultCutParam();

    // The function gets the G-code frame number and returns corresponding index of the bottom and top segments
    std::pair<size_t, size_t> getDxfEntityNum(size_t gframe_num) const {
        if (gframe_num < mapGcodeToContours.size())
            return mapGcodeToContours[gframe_num];
        else if (mapGcodeToContours.size())
            return mapGcodeToContours.back();

        return std::pair<size_t, size_t>({0,0});
    }

    static GCode compile(const QString& gcode);

    void loadBackup();
    bool validGCodeBackup() const { return !gcode.empty(); }
    bool validContextBackup() const { return cncContext.valid(); }
    bool backup();

    static void saveInterfaceLanguage(InterfaceLanguage lang);
    static InterfaceLanguage loadInterfaceLanguage();

    static void saveSwapXY(bool value);
    static bool loadSwapXY();

    static void saveReverseX(bool value);
    static bool loadReverseX();

    static void saveReverseY(bool value);
    static bool loadReverseY();

    static void saveShowXY(bool value);
    static bool loadShowXY();

    static QString helpSubDir();

    static void saveFontSize(int fontSize);
    static int loadFontSize();

    static void saveInputLevel(uint16_t value);
    static uint16_t loadInputLevel(bool& OK);

    static void saveStepDir(bool sdEna);
    static bool loadStepDir(bool& OK);

    static void saveMotorDir(bool revX, bool revY, bool revU, bool revV, bool swapXY, bool swapUV, bool encReverseX, bool encReverseY);
    static void saveFeedbackParam(bool fb_ena, double low_thld[2], double high_thld[2], double rb_to, unsigned rb_attempts, double rb_len, double rb_speed);
    static void saveFeedbackAcceleration(double fb_acc, double fb_dec);
    static void saveAcceleration(bool ena, double acc, double dec);
    static void saveStep(double step, double scaleX, double scaleY, double scaleU, double scaleV, double scaleEncX, double scaleEncY, bool encXY);

    static bool loadMotorReverseX();
    static bool loadMotorReverseY();
    static bool loadMotorReverseU();
    static bool loadMotorReverseV();
    static bool loadMotorSwapXY();
    static bool loadMotorSwapUV();
    static bool loadEncReverseX();
    static bool loadEncReverseY();
    static bool loadFeedbackParam(bool& fb_ena, double (&low_thld)[2], double (&high_thld)[2], double& rb_to, unsigned& rb_attempts, double& rb_len, double& rb_speed);
    static bool loadFeedbackAcceleration(double& fb_acc, double& fb_dec);
    static bool loadAcceleration(bool& ena, double& acc, double& dec);
    static bool loadStep(double& step, double& scaleX, double& scaleY, double& scaleU, double& scaleV, double& scaleEncX, double& scaleEncY, bool& encXY);
    static void loadParam();
    static void loadDefaultParam();

    void saveComb(const comb_t& comb);
    bool loadComb(comb_t& comb);

    void saveSlices(const snake_t& comb);
    bool loadSlices(snake_t& comb);
};

#endif // PROGRAMPARAM_H
