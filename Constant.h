/***********************************************************
**
** 作者：lth
** 日期：2017-07-25
**
** 定义宏常量
**
************************************************************/

#ifndef CONSTANT_H
#define CONSTANT_H

#define PROPERTY_SEQUENCE_NUMBER "SequenceNumber" //序号
//#define PROPERTY_DEVICE_ID      "DeviceID"
#define PROPERTY_PASS_TIME      "PassTime"
#define PROPERTY_lICENCE        "Licence"
#define PROPERTY_BG_COLOR       "BgColor"
#define PROPERTY_RELIABILITY    "Reliability"
#define PROPERTY_SPEED          "Speed"
#define PROPERTY_ACC            "Acc"
#define PROPERTY_LENGTH         "Length"
#define PROPERTY_C_CO2          "C_CO2"
#define PROPERTY_C_CO           "C_CO"
#define PROPERTY_C_HC           "C_HC"
#define PROPERTY_C_NO           "C_NO"
#define PROPERTY_RC_CO2         "RC_CO2"
#define PROPERTY_RC_CO          "RC_CO"
#define PROPERTY_RC_HC          "RC_HC"
#define PROPERTY_RC_NO          "RC_NO"
//#define PROPERTY_DYNAMIC_MODE   "DynamicMode"
#define PROPERTY_VALIDITY       "Validity"
#define PROPERTY_IS_PASS        "IsPass"
#define PROPERTY_VSP            "VSP"
#define PROPERTY_OPACITY        "Opacity" //黑度 = opacityMax / 10.0
#define PROPERTY_WIND_SPEED     "WindSpeed"
#define PROPERTY_WIND_DIRECTION "WindDirection"
#define PROPERTY_OUT_DOOR_TEMP  "OutdoorTemp"
#define PROPERTY_HUMIDITY       "Humidity"
#define PROPERTY_PRESSURE       "Pressure"
#define PROPERTY_RECORD_STATUS  "RecordStatus" //检测状态，合格、超标、无效
#define PROPERTY_PIC            "Pic"
#define PROPERTY_USER_ID        "UserID"  //表格 DB_TABLE_USER 中的主键
#define PROPERTY_SITE_ID        "SiteID"
//#define PROPERTY_SITE_VERSION   "SiteVersion"
#define PROPERTY_EQUIPMENT_ID   "EquipmentID"
#define PROPERTY_IS_DIESEL      "IsDiesel"
#define PROPERTY_OPACITY_PARA   "OpacityPara" //K值 = -log( 1 - ( opacityMax / 10 ) ) * 0.1
#define PROPERTY_OPACITY_MAX    "OpacityMax"
#define PROPERTY_OPACITY_AVG    "OpacityAvg"

#define PROPERTY_USER_NAME          "Name"
#define PROPERTY_USER_CHINESE_NAME  "ChineseName"
#define PROPERTY_USER_DESCRIPTION   "Description"

#define PROPERTY_SITE_NAME          "Name"
#define PROPERTY_SITE_LATITUDE      "Latitude"
#define PROPERTY_SITE_LONGITUDE     "Longitude"
#define PROPERTY_SITE_GRADIENT      "Gradient"

#define DB_TABLE_TESTDATA       "TestData"
#define DB_TABLE_USER           "User"
#define DB_TABLE_SITE           "Site"

#define CAPTURE_CAR_PIC_WIDTH   1920.0
#define CAPTURE_CAR_PIC_HEIGHT  1112.0

#define RESULT_SAVE_PATH        "Results/"
#define TEMPLATE_RESULT_PATH    "Results/data/"



#define TEST_RESULT_FILE_NAME(var) (QString("Data-%1.mdb").arg(var))

#define ERROR_NUMBER            -999
#define INIT_NUMBER             -1  //初始化数据
#define ERROR_INFO              "UNDEFINED"


#define LOG_FILE_PATH       QString("log/") //日志文件存储路径
#define SETTINGS_FILE_PATH  QString("config/Settings.ini") //设置、用户、监测地点配置文件
#define USER_FILE_PATH      QString("config/User.ini")     //用户管理
#define SITE_FILE_PATH      QString("config/Site.ini")     //地点管理

#define HKEY_CAMERA_IP        "Camera/ip"
#define HKEY_CAMERA_PORT      "Camera/port"
#define HKEY_CAMERA_USER      "Camera/user"
#define HKEY_CAMERA_PASSWORD  "Camera/password"

#define HKEY_PLATE_RECOG_REGION_TOP    "Common/plate_recog_region_top"
#define HKEY_PLATE_RECOG_REGION_RIGHT  "Common/plate_recog_region_right"
#define HKEY_WEATHER_COM               "Common/weather_com"
#define HKEY_IS_SAVING_OUTER_REGIN_CAR "Common/is_saving_outer_regin_car"
#define HKEY_RESULTS_PATH              "Common/results_path"

#define HKEY_LIMIT_HC                 "Limit/hc"
#define HKEY_LIMIT_NO                 "Limit/no"
#define HKEY_LIMIT_CO                 "Limit/co"
#define HKEY_LIMIT_K                  "Limit/K"
#define HKEY_LIMIT_OPACITY            "Limit/opacity"

#define HKEY_PREFERENCE_THRESHOLD_DIESEL_OPACITY "Preference/threshold_diesel_opacity"
#define HKEY_PREFERENCE_ENABLE_ACC_CHECK         "Preference/enable_acc_check"
#define HKEY_PREFERENCE_THRESHOLD_ACC_LOWER      "Preference/threshold_acc_lower"
#define HKEY_PREFERENCE_ENABLE_VSP_CHECK         "Preference/enable_VSP_check"
#define HKEY_PREFERENCE_THRESHOLD_VSP_LOWER      "Preference/threshold_VSP_lower"
#define HKEY_PREFERENCE_THRESHOLD_VSP_UPPER      "Preference/threshold_VSP_upper"

#define HKEY_CaliData_HC         "CaliData/hc"
#define HKEY_CaliData_NO         "CaliData/no"
#define HKEY_CaliData_CO         "CaliData/co"
#define HKEY_CaliData_CO2        "CaliData/co2"

#define SOFTWARE_NAME QString("YaoGan")

#define SENTENCE_TO_DELETE //用于提醒待删除的语句（一些临时加的调试语句） 160825
#define SENTENCE_TO_PROCESS //用于提醒哪些句子需要后续处理的，有可能临时注释掉了，或改变了参数 160825

#endif // CONSTANT_H

