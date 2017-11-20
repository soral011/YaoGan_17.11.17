#include "AddUserDialog.h"
#include "ui_AddUserDialog.h"
#include "stdlib.h"
#include "Global.h"

AddUserDialog::AddUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddUserDialog)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    g_connectButtonSignal(this);

    g_clearPushButtonFocus(this);
}

AddUserDialog::~AddUserDialog()
{
    delete ui;
}

void AddUserDialog::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_ok")
    {
        UserStruct user;
        user.m_userName = ui->lineEdit_user_name->text();
        user.m_userPassword = ui->lineEdit_password->text();
        user.m_userChineseName = ui->lineEdit_chinese_name->text();
        user.m_userDiscription = ui->lineEdit_user_description->text();

        bool isValid = isUserValid(user);

        if(isValid)
        {
            user.m_userPassword = g_codeMd5(user.m_userPassword);
            this->close();

            emit addUserReady(user);
        }
    }
    else if(buttonName == "pushButton_close" || buttonName == "pushButton_cancel")
    {
        g_closeWithAnim(this);
    }
}

bool AddUserDialog::isUserValid(UserStruct user)
{
    //用户名
    QString userName = user.m_userName;
    if(userName.isEmpty() || userName.remove(" ").isEmpty())
    {
        g_showTipsDialog(tr("用户名不能为空。"),  this);
        return false;
    }

    userName = user.m_userName;
    userName.remove("_");
    userName.remove(QRegExp("[A-Z]"));
    userName.remove(QRegExp("[a-z]"));
    userName.remove(QRegExp("[0-9]"));
    if(userName.count())
    {
        g_showTipsDialog(tr("用户名只能包含数字、字母和下划线。"),  this);
        return false;
    }

    if(user.m_userName.count() < 3)
    {
        g_showTipsDialog(tr("用户名长度不能小于3。"),  this);
        return false;
    }

    //密码
    bool isOk = isPasswordValid();
    if(!isOk)
    {
        return isOk;
    }

    //用户姓名
    isOk = isChineseNameValid(user.m_userChineseName);
    if(!isOk)
    {
        g_showTipsDialog(tr("用户姓名不合法。"),  this);
        return false;
    }

    //用户描述
//    QString text = user.m_userName;
//    if(text.isEmpty() || text.remove(" ").isEmpty())
//    {
//        g_showTipsDialog(tr("用户描述不能为空"),  this);
//        return false;
//    }

    return true;
}

bool AddUserDialog::isPasswordValid()
{

    QString password = ui->lineEdit_password->text();
    QString confirmPassword = ui->lineEdit_password_confirm->text();
    if(password != confirmPassword)
    {
        g_showTipsDialog(tr("密码不匹配。"), this);
        return false;
    }

    QStringList passwordList;
    passwordList << password << confirmPassword;
    foreach(QString pass, passwordList)
    {
        if(pass.isEmpty() || pass.remove(" ").isEmpty())
        {
            g_showTipsDialog(tr("密码不能为空。"), this);
            return false;
        }

        QString tmpPassword = pass;
        tmpPassword.remove(QRegExp("[0-9]"));
        tmpPassword.remove(QRegExp("[a-z]"));
        tmpPassword.remove(QRegExp("[A-Z]"));
        if(tmpPassword.count())
        {
            g_showTipsDialog(tr("密码只能包含字母和数字。"),  this);
            return false;
        }
        if(pass.count() < 6)
        {
            g_showTipsDialog(tr("密码长度不能小于6。"),  this);
            return false;
        }
    }

    return true;
}

void AddUserDialog::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void AddUserDialog::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos() - event->globalPos();
}

void AddUserDialog::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() + m_relativePos);
}


bool AddUserDialog::isChineseNameValid(QString chineseName)
{
    int FAMILY_NAMES_SINGLE_CNT = 444;  // 单姓
    int FAMILY_NAMES_CNT = 504;         // 百家姓

    QString szBookOfFamilyNames[FAMILY_NAMES_CNT] = {
        "赵", "钱", "孙", "李", "周", "吴", "郑", "王", "冯", "陈",
        "褚", "卫", "蒋", "沈", "韩", "杨", "朱", "秦", "尤", "许",
        "何", "吕", "施", "张", "孔", "曹", "严", "华", "金", "魏",
        "陶", "姜", "戚", "谢", "邹", "喻", "柏", "水", "窦", "章",
        "云", "苏", "潘", "葛", "奚", "范", "彭", "郎", "鲁", "韦",
        "昌", "马", "苗", "凤", "花", "方", "俞", "任", "袁", "柳",
        "酆", "鲍", "史", "贺", "唐", "费", "廉", "岑", "薛", "雷",
        "倪", "汤", "滕", "殷", "罗", "毕", "郝", "邬", "安", "常",
        "乐", "于", "时", "傅", "皮", "卞", "齐", "康", "伍", "余",
        "元", "卜", "顾", "孟", "平", "黄", "和", "穆", "萧", "尹",
        "姚", "邵", "湛", "汪", "祁", "毛", "禹", "狄", "米", "贝",
        "明", "臧", "计", "伏", "成", "戴", "谈", "宋", "茅", "庞",
        "熊", "纪", "舒", "屈", "项", "祝", "董", "粱", "杜", "阮",
        "蓝", "闵", "席", "季", "麻", "强", "贾", "路", "娄", "危",
        "江", "童", "颜", "郭", "梅", "盛", "林", "刁", "钟", "徐",
        "邱", "骆", "高", "夏", "蔡", "田", "樊", "胡", "凌", "霍",
        "虞", "万", "支", "柯", "昝", "管", "卢", "莫", "经", "房",
        "裘", "缪", "干", "解", "应", "宗", "丁", "宣", "贲", "邓",
        "郁", "单", "杭", "洪", "包", "诸", "左", "石", "崔", "吉",
        "钮", "龚", "程", "嵇", "邢", "滑", "裴", "陆", "荣", "翁",
        "荀", "羊", "於", "惠", "甄", "麴", "家", "封", "芮", "羿",
        "储", "靳", "汲", "邴", "糜", "松", "井", "段", "富", "巫",
        "乌", "焦", "巴", "弓", "牧", "隗", "山", "谷", "车", "侯",
        "宓", "蓬", "全", "郗", "班", "仰", "秋", "仲", "伊", "宫",
        "宁", "仇", "栾", "暴", "甘", "钭", "厉", "戎", "祖", "武",
        "符", "刘", "景", "詹", "束", "龙", "叶", "幸", "司", "韶",
        "郜", "黎", "蓟", "薄", "印", "宿", "白", "怀", "蒲", "邰",
        "从", "鄂", "索", "咸", "籍", "赖", "卓", "蔺", "屠", "蒙",
        "池", "乔", "阴", "欎", "胥", "能", "苍", "双", "闻", "莘",
        "党", "翟", "谭", "贡", "劳", "逄", "姬", "申", "扶", "堵",
        "冉", "宰", "郦", "雍", "舄", "璩", "桑", "桂", "濮", "牛",
        "寿", "通", "边", "扈", "燕", "冀", "郏", "浦", "尚", "农",
        "温", "别", "庄", "晏", "柴", "瞿", "阎", "充", "慕", "连",
        "茹", "习", "宦", "艾", "鱼", "容", "向", "古", "易", "慎",
        "戈", "廖", "庾", "终", "暨", "居", "衡", "步", "都", "耿",
        "满", "弘", "匡", "国", "文", "寇", "广", "禄", "阙", "东",
        "殴", "殳", "沃", "利", "蔚", "越", "夔", "隆", "师", "巩",
        "厍", "聂", "晁", "勾", "敖", "融", "冷", "訾", "辛", "阚",
        "那", "简", "饶", "空", "曾", "毋", "沙", "乜", "养", "鞠",
        "须", "丰", "巢", "关", "蒯", "相", "查", "後", "荆", "红",
        "游", "竺", "权", "逯", "盖", "益", "桓", "公", "墨", "哈",
        "谯", "笪", "年", "爱", "阳", "佟", "商", "帅", "佘", "佴",
        "仉", "督", "归", "海", "伯", "赏", "岳", "楚", "缑", "亢",
        "况", "后", "有", "琴", "言", "福", "晋", "牟", "闫", "法",
        "汝", "鄢", "涂", "钦", "东郭", "南门", "呼延", "羊舌", "微生", "左丘",
        "万俟", "司马", "上官", "欧阳", "夏侯", "诸葛", "闻人", "东方", "赫连", "皇甫",
        "尉迟", "公羊", "澹台", "公冶", "宗政", "濮阳", "东门", "西门", "南宫", "第五",
        "淳于", "单于", "太叔", "申屠", "公孙", "仲孙", "轩辕", "令狐", "钟离", "宇文",
        "长孙", "慕容", "鲜于", "闾丘", "司徒", "司空", "亓官", "司寇", "子车", "夹谷",
        "颛孙", "端木", "巫马", "公西", "漆雕", "乐正", "壤驷", "公良", "拓跋", "梁丘",
        "宰父", "谷梁", "段干", "百里"};


    // 是否全中文
    for(int i = 0; i < chineseName.count(); i++)
    {
        QString str(chineseName.at(i));
        bool value = str.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
        if(!value)
        {
            MY_DEBUG("");
            return false;
        }
    }

    QString strName, strName1, strName2;
    strName = chineseName;

    strName1 = strName.left(1); // 单姓
    strName2 = strName.left(2); // 复姓

    for (int i = 0; i < FAMILY_NAMES_SINGLE_CNT; i++)
    {
        if (strName1 == szBookOfFamilyNames[i] && strName.count() > 1)
        {
            return true;
        }
    }

    for(int k = FAMILY_NAMES_SINGLE_CNT; k < FAMILY_NAMES_CNT; k++)
    {
        if (strName2 == szBookOfFamilyNames[k]  && strName.count() > 2)
        {
            return true;
        }
    }

    MY_DEBUG("");
    return FALSE;
}


void AddUserDialog::showEvent(QShowEvent *event)
{
    ui->lineEdit_chinese_name->clear();
    ui->lineEdit_password->clear();
    ui->lineEdit_password_confirm->clear();
    ui->lineEdit_user_description->clear();
    ui->lineEdit_user_name->clear();
}

QString AddUserDialog::getUserMD5(UserStruct user)
{
    QString userID = g_codeMd5( user.m_userChineseName
                                 + user.m_userDiscription
                                 + user.m_userName
                                 + user.m_userPassword);
    return userID;
}
