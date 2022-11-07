#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <QObject>
#include <QQmlEngine>
#include <QVector>

#include "coreengine/console.h"
#include "coreengine/mainapp.h"

#include "3rd_party/oxygine-framework/oxygine/core/intrusive_ptr.h"

class Interpreter;
using spInterpreter = oxygine::intrusive_ptr<Interpreter>;
/**
 * @brief The Interpreter class java-script interpreter with easy access functions
 */
class Interpreter final : public QQmlEngine, public oxygine::ref_counter
{
    Q_OBJECT

public:
    static Interpreter* getInstance()
    {
        return m_pInstance.get();
    }
    static Interpreter* createInstance()
    {
        if (m_pInstance.get() == nullptr)
        {
            m_pInstance = spInterpreter::create();
            m_pInstance->init();
        }
        else
        {
            oxygine::handleErrorPolicy(oxygine::ep_show_error, "illegal interpreter creation");
        }
        return m_pInstance.get();
    }
    ~Interpreter();
    static void release();

    static void setCppOwnerShip(QObject* object);
    /**
     * @brief getRuntimeData
     * @return
     */
    static QString getRuntimeData();
    /**
     * @brief reloadInterpreter
     * @param runtime
     */
    static bool reloadInterpreter(const QString & runtime);

    bool getInJsCall() const;

    template<typename _TType, template<typename T> class _TVectorList>
    QJSValue arraytoJSValue(const _TVectorList<_TType> & array)
    {
        QJSValue jsArray = newArray(array.size());
        for (qint32 i = 0; i < array.size(); i++)
        {
            if constexpr (std::is_same<_TType, QPoint>::value)
            {
                jsArray.setProperty(i, toScriptValue(array[i]));
            }
            else
            {
                jsArray.setProperty(i, array[i]);
            }
        }
        return jsArray;
    }

signals:
    void sigNetworkGameFinished(qint32 value, QString id);
public slots:
    bool openScript(const QString & script, bool setup);
    bool loadScript(const QString & content, const QString & script);
    inline QJSValue doFunction(const QString & func, const QJSValueList& args = QJSValueList())
    {
        QJSValue ret;
        QJSValue funcPointer = globalObject().property(func);
#ifdef GAMEDEBUG
        OXY_ASSERT(Mainapp::getInstance()->getWorkerthread() == QThread::currentThread());
        if (funcPointer.isCallable())
        {
#endif
            ++m_inJsCall;
            ret = funcPointer.call(args);
            exitJsCall();
            if (ret.isError())
            {
                QString error = ret.toString() + " in File: " +
                                ret.property("fileName").toString() + " at Line: " +
                                ret.property("lineNumber").toString();
                CONSOLE_PRINT(error, Console::eERROR);
            }
#ifdef GAMEDEBUG
        }
        else
        {
            QString error = "Error: attemp to call a non function value. Call:" + func;
            CONSOLE_PRINT(error, Console::eERROR);
        }
#endif
        return ret;
    }
    inline QJSValue doFunction(const QString & obj, const QString & func, const QJSValueList& args = QJSValueList())
    {
        QJSValue ret;
        QJSValue objPointer = globalObject().property(obj);
#ifdef GAMEDEBUG
        OXY_ASSERT(Mainapp::getInstance()->getWorkerthread() == QThread::currentThread());
        if (objPointer.isObject())
        {
#endif
            QJSValue funcPointer = objPointer.property(func);
#ifdef GAMEDEBUG
            if (funcPointer.isCallable())
            {
#endif
                ++m_inJsCall;
                ret = funcPointer.call(args);
                exitJsCall();
                if (ret.isError())
                {
                    QString error = ret.toString() + " in File: " +
                                    ret.property("fileName").toString() + " at Line: " +
                                    ret.property("lineNumber").toString();
                    CONSOLE_PRINT(error, Console::eERROR);
                }
#ifdef GAMEDEBUG
            }
            else
            {
                QString error = "Error: attemp to call a non function value. Call:" + obj + "." + func;
                CONSOLE_PRINT(error, Console::eERROR);
            }
        }
        else
        {
            QString error = "Error: attemp to call a non object value in order to call a function. Call:" + obj + "." + func;
            CONSOLE_PRINT(error, Console::eERROR);
        }
#endif
        return ret;
    }
    void cleanMemory();
    /**
     * @brief doString immediatly interprates the string with the javascript-interpreter
     * @param task string parsed to the interpreter
     */
    QJSValue doString(const QString & task);
    /**
     * @brief pushInt
     * @param value
     */
    void pushInt(const QString & name, qint32 value);
    void pushDouble(const QString & name, double value);
    void pushString(const QString & name, const QString & value);
    void pushObject(const QString & name, QObject* object);
    inline QJSValue newQObject(QObject* object)
    {
        return QQmlEngine::newQObject(object);
    }
    void deleteObject(const QString & name);

    qint32 getGlobalInt(const QString & var);
    bool getGlobalBool(const QString & var);
    double getGlobalDouble(const QString & var);
    QString getGlobalString(const QString & var);
    QJSValue getGlobal(const QString & var);
    void setGlobal(const QString & var, const QJSValue & obj);
    /**
     * @brief exists checks if the js object exists
     * @param object
     * @return
     */
    bool exists(const QString & object)
    {
        QJSValue objPointer = globalObject().property(object);
        if (objPointer.isObject())
        {
            return true;
        }
        return false;
    }
    /**
     * @brief exists checks if the js object and function exists
     * @param object
     * @return
     */
    bool exists(const QString & object, const QString & function)
    {
        QJSValue objPointer = globalObject().property(object);
        if (objPointer.isObject())
        {
            QJSValue funcPointer = objPointer.property(function);
            if (funcPointer.isCallable())
            {
                return true;
            }
        }
        return false;
    }
    void trackJsObject(oxygine::ref_counter* pObj);

private slots:
    void networkGameFinished(qint32 value, QString id);
private:
    friend class oxygine::intrusive_ptr<Interpreter>;
    explicit Interpreter();    
    /**
     * @brief init
     */
    void init();
    void exitJsCall();
private:
    static QString m_runtimeData;
    static spInterpreter m_pInstance;
    qint32 m_inJsCall{0};
    QVector<oxygine::intrusive_ptr<oxygine::ref_counter>> m_jsObjects;
};

#endif // INTERPRETER_H
