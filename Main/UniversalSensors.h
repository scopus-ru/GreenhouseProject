#ifndef _UNUVERSAL_SENSORS_H
#define _UNUVERSAL_SENSORS_H
#include <Arduino.h>
#include "ModuleController.h"
#include "TinyVector.h"
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// команды
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#define UNI_START_MEASURE 0x44 // запустить конвертацию
#define UNI_READ_SCRATCHPAD 0xBE // прочитать скратчпад
#define UNI_WRITE_SCRATCHPAD  0x4E // записать скратчпад
#define UNI_SAVE_EEPROM 0x25 // сохранить настройки в EEPROM

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// максимальное кол-во датчиков в универсальном модуле
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_UNI_SENSORS 3
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// значение, говорящее, что датчика нет
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#define NO_SENSOR_REGISTERED 0xFF
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// описание разных частей скратчпада
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte packet_type; // тип пакета
  byte packet_subtype; // подтип пакета
  byte config; // конфигурация
  byte controller_id; // ID контроллера, к которому привязан модуль
  byte rf_id; // идентификатор RF-канала модуля
  
} UniScratchpadHead; // голова скратчпада, общая для всех типов модулей
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  UniScratchpadHead head; // голова
  byte data[24]; // сырые данные
  byte crc8; // контрольная сумма
  
} UniRawScratchpad; // "сырой" скратчпад, байты данных могут меняться в зависимости от типа модуля
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
  slotEmpty, // пустой слот, без настроек
  slotWindowLeftChannel, // настройки привязки к левому каналу одного окна
  slotWindowRightChannel, // настройки привязки к правому каналу одного окна
  slotWateringChannel, // настройки привязки к статусу канала полива 
  slotLightChannel, // настройки привязки к статусу канала досветки
  slotPin // настройки привязки к статусу пина
  
} UniSlotType; // тип слота, для которого указаны настройки
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte slotType; // тип слота, одно из значений UniSlotType 
  byte slotLinkedData; // данные, привязанные к слоту мастером, должны хранится слейвом без изменений
  byte slotStatus; // статус слота (HIGH или LOW)
  
} UniSlotData; // данные одного слота настроек универсального модуля 
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  UniSlotData slots[8]; // слоты настроек
  
} UniExecutionModuleScratchpad; // скратчпад исполнительного модуля
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte index; // индекс датчика
  byte type; // тип датчика
  byte data[4]; // данные датчика
  
} UniSensorData; // данные с датчика универсального модуля
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte battery_status;
  byte calibration_factor1;
  byte calibration_factor2;
  byte query_interval_min;
  byte query_interval_sec;
  byte reserved;
  UniSensorData sensors[MAX_UNI_SENSORS];
   
} UniSensorsScratchpad; // скратчпад модуля с датчиками
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte sensorType; // тип датчика
  byte sensorData[2]; // данные датчика
   
} UniNextionData; // данные для отображения на Nextion
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  uint8_t reserved[3]; // резерв, добитие до 24 байт
  uint8_t controllerStatus;
  uint8_t nextionStatus1;
  uint8_t nextionStatus2;  
  uint8_t openTemperature; // температура открытия окон
  uint8_t closeTemperature; // температура закрытия окон

  uint8_t dataCount; // кол-во записанных показаний с датчиков
  UniNextionData data[5]; // показания с датчиков
  
} UniNextionScratchpad; // скратчпад выносного модуля Nextion
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс для работы со скратчпадом, представляет основные функции, никак не изменяет переданный скратчпад до вызова функции read и функции write.
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class UniScratchpadClass
{
  public:
    UniScratchpadClass();

    void begin(byte pin,UniRawScratchpad* scratch); // привязываемся к пину и куску памяти, куда будем писать данные
    bool read(); // читаем скратчпад
    bool write(); // пишем скратчпад
    bool save(); // сохраняем скратчпад в EEPROM модуля
    bool startMeasure(); // запускаем конвертацию

  private:

    byte pin;
    UniRawScratchpad* scratchpad;

    bool canWork(); // проверяем, можем ли работать
  
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
/*

Все неиспользуемые поля инициализируются 0xFF

структура скратчпада модуля с датчиками:

packet_type - 1 байт, тип пакета (прописано значение 1)
packet_subtype - 1 байт, подтип пакета (прописано значение 0)
config - 1 байт, конфигурация (бит 0 - вкл/выкл передатчик, бит 1 - поддерживается ли фактор калибровки)
controller_id - 1 байт, идентификатор контроллера, к которому привязан модуль
rf_id - 1 байт, уникальный идентификатор модуля
battery_status - 1 байт, статус заряда батареи
calibration_factor1 - 1 байт, фактор калибровки
calibration_factor2 - 1 байт, фактор калибровки
query_interval - 1 байт, интервал обновления показаний (старшие 4 бита - минуты, младшие 4 бита - секунды)
reserved - 2 байт, резерв
index1 - 1 байт, индекс первого датчика в системе
type1 - 1 байт, тип первого датчика
data1 - 4 байта, данные первого датчика
index2 - 1 байт
type2 - 1 байт
data2 - 4 байта
index3 - 1 байт
type3 - 1 байт
data3 - 4 байта
crc8 - 1 байт, контрольная сумма скратчпада

*/
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
  uniNone = 0, // ничего нет
  uniTemp = 1, // только температура, значащие - два байта
  uniHumidity = 2, // влажность (первые два байта), температура (вторые два байта) 
  uniLuminosity = 3, // освещённость, 4 байта
  uniSoilMoisture = 4, // влажность почвы (два байта)
  uniPH = 5 // pH (два байта)
  
} UniSensorType; // тип датчика
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
 OneState* State1; // первое внутреннее состояние в контроллере
 OneState* State2; // второе внутреннее состояние в контроллере  
 
} UniSensorState; // состояние для датчика, максимум два (например, для влажности надо ещё и температуру тянуть)
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
  uniSensorsClient = 1, // packet_type == 1
  uniNextionClient = 2, // packet_type == 2
  uniExecutionClient = 3 // packet_type == 3
  
} UniClientType; // тип клиента
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс поддержки регистрации универсальных датчиков в системе
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class UniRegDispatcher 
 {
  public:
    UniRegDispatcher();

    void Setup(); // настраивает диспетчер регистрации перед работой

    // добавляет тип универсального датчика в систему, возвращает true, если добавилось (т.е. индекс был больше, чем есть в системе)
    bool AddUniSensor(UniSensorType type, uint8_t sensorIndex);

    // возвращает состояния для ранее зарегистрированного датчика
    bool GetRegisteredStates(UniSensorType type, uint8_t sensorIndex, UniSensorState& resultStates);

    // возвращает кол-во жёстко прописанных в прошивке датчиков того или иного типа
    uint8_t GetHardCodedSensorsCount(UniSensorType type); 
    // возвращает кол-во зарегистрированных универсальных модулей нужного типа
    uint8_t GetUniSensorsCount(UniSensorType type);

    uint8_t GetControllerID(); // возвращает уникальный ID контроллера

    void SaveState(); // сохраняет текущее состояние

    uint8_t GetRFChannel(); // возвращает текущий канал для nRF
    void SetRFChannel(uint8_t channel); // устанавливает канал для nRF


 private:

    void ReadState(); // читает последнее запомненное состояние
    void RestoreState(); // восстанавливает последнее запомненное состояние


    // модули разного типа, для быстрого доступа к ним
    AbstractModule* temperatureModule; // модуль температуры
    AbstractModule* humidityModule; // модуль влажности
    AbstractModule* luminosityModule; // модуль освещенности
    AbstractModule* soilMoistureModule; // модуль влажности почвы
    AbstractModule* phModule; // модуль контроля pH

    // жёстко указанные в прошивке датчики
    uint8_t hardCodedTemperatureCount;
    uint8_t hardCodedHumidityCount;
    uint8_t hardCodedLuminosityCount;
    uint8_t hardCodedSoilMoistureCount;
    uint8_t hardCodedPHCount;


    // последние выданные индексы для универсальных датчиков
    uint8_t currentTemperatureCount;
    uint8_t currentHumidityCount;
    uint8_t currentLuminosityCount;
    uint8_t currentSoilMoistureCount;
    uint8_t currentPHCount;

    uint8_t rfChannel; // номер канала для nRF
  
 };
//-------------------------------------------------------------------------------------------------------------------------------------------------------
extern UniRegDispatcher UniDispatcher; // экземпляр класса диспетчера, доступный отовсюду
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// абстрактный класс клиента, работающего с универсальным модулем по шине 1-Wire
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
    ssOneWire // с линии 1-Wire
  , ssRadio // по радиоканаду
  
} UniScratchpadSource; // откуда был получен скратчпад
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class AbstractUniClient
{
    public:
      AbstractUniClient() {};

      // регистрирует модуль в системе, если надо - прописывает индексы виртуальным датчикам и т.п.
      virtual void Register(UniRawScratchpad* scratchpad) = 0; 

      // обновляет данные с модуля, в receivedThrough - откуда был получен скратчпад
      virtual void Update(UniRawScratchpad* scratchpad, bool isModuleOnline, UniScratchpadSource receivedThrough) = 0;

      void SetPin(byte p) { pin = p; }

   protected:
      byte pin;
  
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс ничего не делающего клиента
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class DummyUniClient : public AbstractUniClient
{
  public:
    DummyUniClient() : AbstractUniClient() {}
    virtual void Register(UniRawScratchpad* scratchpad) { UNUSED(scratchpad); }
    virtual void Update(UniRawScratchpad* scratchpad, bool isModuleOnline, UniScratchpadSource receivedThrough) { UNUSED(scratchpad); UNUSED(isModuleOnline); UNUSED(receivedThrough); }
  
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс клиента для работы с модулями датчиков
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class SensorsUniClient : public AbstractUniClient
{
  public:
    SensorsUniClient();
    virtual void Register(UniRawScratchpad* scratchpad);
    virtual void Update(UniRawScratchpad* scratchpad, bool isModuleOnline, UniScratchpadSource receivedThrough);

  private:

    unsigned long measureTimer;
    void UpdateStateData(const UniSensorState& states,const UniSensorData* data,bool IsModuleOnline);
    void UpdateOneState(OneState* os, const UniSensorData* data, bool IsModuleOnline);
  
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс клиента исполнительного модуля
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_UNI_EXECUTION_MODULE
class UniExecutionModuleClient  : public AbstractUniClient
{
  public:

    UniExecutionModuleClient();
    virtual void Register(UniRawScratchpad* scratchpad);
    virtual void Update(UniRawScratchpad* scratchpad, bool isModuleOnline, UniScratchpadSource receivedThrough);
  
};
#endif
//-------------------------------------------------------------------------------------------------------------------------------------------------------
struct UniNextionWaitScreenData
{
  byte sensorType;
  byte sensorIndex;
  const char* moduleName;
  UniNextionWaitScreenData(byte a, byte b, const char* c) 
  {
    sensorType = a;
    sensorIndex = b;
    moduleName = c;
  }
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_UNI_NEXTION_MODULE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class NextionUniClient : public AbstractUniClient
{
  public:
    NextionUniClient();
    virtual void Register(UniRawScratchpad* scratchpad);
    virtual void Update(UniRawScratchpad* scratchpad, bool isModuleOnline, UniScratchpadSource receivedThrough);

  private:

    unsigned long updateTimer;
    bool tempChanged;
  
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif // USE_UNI_NEXTION_MODULE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_RS485_GATE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
enum {RS485FromMaster = 1, RS485FromSlave = 2};
enum {RS485ControllerStatePacket = 1, RS485SensorDataPacket = 2};
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte header1;
  byte header2;

  byte direction; // направление: 1 - от меги, 2 - от слейва
  byte type; // тип: 1 - пакет исполнительного модуля, 2 - пакет модуля с датчиками

  byte data[sizeof(ControllerState)]; // N байт данных, для исполнительного модуля в этих данных содержится состояние контроллера
  // для модуля с датчиками: первый байт - тип датчика, 2 байт - его индекс в системе. В обратку модуль с датчиками должен заполнить показания (4 байта следом за индексом 
  // датчика в системе и отправить пакет назад, выставив direction и type.

  byte tail1;
  byte tail2;
  byte crc8; // контрольная сумма пакета
  
} RS485Packet; // пакет, гоняющийся по RS-485 туда/сюда (21 байт)
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte sensorType; // тип датчика
  byte sensorIndex; // зарегистрированный в системе индекс
  
} RS485QueueItem; // запись в очереди на чтение показаний из шины
//----------------------------------------------------------------------------------------------------------------
typedef Vector<RS485QueueItem> RS485Queue; // очередь к опросу
//----------------------------------------------------------------------------------------------------------------
class UniRS485Gate // класс для работы универсальных модулей через RS-485
{
  public:
    UniRS485Gate();
    void Setup();
    void Update(uint16_t dt);

  private:
#ifdef USE_UNI_EXECUTION_MODULE
    unsigned long updateTimer;
#endif    

    void waitTransmitComplete();
    void enableSend();
    void enableReceive();
    byte crc8(const byte *addr, byte len);

  #ifdef USE_UNIVERSAL_SENSORS // если комплимся с поддержкой модулей с датчиками - тогда обрабатываем очередь

    bool isInOnlineQueue(const RS485QueueItem& item);
    RS485Queue sensorsOnlineQueue; // очередь датчиков, с которых были показания
    RS485Queue queue;
    byte currentQueuePos;
    unsigned long sensorsTimer;
  #endif  
    
};

extern UniRS485Gate RS485;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif // USE_RS485_GATE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_NRF_GATE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс работы с универсальными модулями через радиоканал
//-------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte controller_id; // ID контроллера, который выплюнул в эфир пакет
  ControllerState state; // состояние контроллера
  byte reserved[14]; // резерв, добитие до 30 байт
  byte crc8; // контрольная сумма
  
} NRFControllerStatePacket; // пакет с состоянием контроллера
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  
  byte sensorType; // тип датчика
  byte sensorIndex; // зарегистрированный в системе индекс
  uint16_t queryInterval; // интервал между получениями информации с датчика
  unsigned long gotLastDataAt; // колда были получены последние данные
  
} NRFQueueItem;
//----------------------------------------------------------------------------------------------------------------
typedef Vector<NRFQueueItem> NRFQueue;
//----------------------------------------------------------------------------------------------------------------
class UniNRFGate
{
  public:
    UniNRFGate();
    void Setup();
    void Update(uint16_t dt);

    void SetChannel(byte channel);

  private:
  
    void initNRF();
    void readFromPipes();
    
    bool bFirstCall;
    NRFControllerStatePacket packet;
    bool nRFInited;

    NRFQueue sensorsOnlineQueue;
    bool isInOnlineQueue(byte sensorType,byte sensorIndex, byte& result_index);
  
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif // USE_NRF_GATE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// Фабрика клиентов
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class UniClientsFactory
{
  private:

    DummyUniClient dummyClient;
    SensorsUniClient sensorsClient;
    #ifdef USE_UNI_NEXTION_MODULE
    NextionUniClient nextionClient;
    #endif

    #ifdef USE_UNI_EXECUTION_MODULE
    UniExecutionModuleClient executionClient;
    #endif
  
  public:
    UniClientsFactory();
    // возвращает клиента по типу пакета скратчпада
    AbstractUniClient* GetClient(UniRawScratchpad* scratchpad);
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
extern UniClientsFactory UniFactory; // наша фабрика клиентов
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#if UNI_WIRED_MODULES_COUNT > 0
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс опроса универсальных модулей, постоянно висящих на линии
//-------------------------------------------------------------------------------------------------------------------------------------------------------
 class UniPermanentLine
 {
  public:
    UniPermanentLine(uint8_t pinNumber);

    void Update(uint16_t dt);

  private:


    bool IsRegistered();

    AbstractUniClient* lastClient; // последний известный клиент
    byte pin;
    unsigned long timer; // таймер обновления
  
 };
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// класс регистрации универсальных модулей в системе 
//-------------------------------------------------------------------------------------------------------------------------------------------------------
class UniRegistrationLine
{
  public:
    UniRegistrationLine(byte pin);

    bool IsModulePresent(); // проверяет, есть ли модуль на линии
    void CopyScratchpad(UniRawScratchpad* dest); // копирует скратчпад в другой

    bool SetScratchpadData(UniRawScratchpad* src); // копирует данные из переданного скратчпада во внутренний

    void Register(); // регистрирует универсальный модуль в системе

  private:

    bool IsSameScratchpadType(UniRawScratchpad* src); // тестирует, такой же тип скратчпада или нет

    // скратчпад модуля на линии, отдельный, т.к. регистрация у нас разнесена по времени с чтением скратчпада,
    // и поэтому мы не можем здесь использовать общий скратчпад/
    UniRawScratchpad scratchpad; 

    // пин, на котором мы висим
    byte pin;
  
};
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif
