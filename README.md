# SQLite3-ICU

#### 目的
基于ICU构造SQLite3中文分词器，用于Android App全文检索。  

#### 参考列表
1. http://www-01.ibm.com/software/globalization/icu/index.html，官方介绍， 中文分词器用到`Analysis`特性；  
2. https://github.com/android/platform_external_icu4c， Android内置ICU源码，不同Android版本内置不同版本ICU，不同版本ICU，源码不兼容；  
3. http://chenggoi.com/2015/01/06/Android_ICU_Customizing/，《Android 4.4.2 ICU 语言包 精简、裁剪、定制、本地化》，推荐文章；  
4. http://packages.ubuntu.com/trusty/libicu-dev，14.04 apt-get icu52.1版本，但为了和Android 4.4.2 ICU49.1.1对齐，采用源码编译；  

#### 