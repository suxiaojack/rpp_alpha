### RPP - 开源编程语言


##### 为什么需要它？

理由一：RPP既可以编译运行又可以解释运行，既可以用静态类型又可以用动态类型。语法层支持C++、JS、Lisp、python、asm混合编程，一种语言，五种语法。目前RPP兼容60%的C++语法，75%的C语法，并且支持自举（自我编译）。

理由二：它以Lisp作为中间层，编译器在运行期可用，程序在运行时可以改变其结构，新的函数可以被引进，因此支持mixin、元编程以及各种动态特性，并且同时支持call_by_name、call_by_need和call_by_value。

理由三：RPP以静态类型为主，没有GC，所以理论运行速度和C++一样，最终的效率肯定比动态类型的lua要高，并且不会引起GC停顿。目前RPP运行速度与luaJIT接近，比tcc略快，并且RPP支持翻译为C++（可达到与C++接近的速度）。

理由四：RPP的设计目标是简洁、快速、稳定。它的源码结构比lua简单得多，但实现的功能不比lua少，因此RPP将是您实现编译器的绝佳参考。另外嵌入编译器只需要包含一个头文件即可（zmain.h），您再也不用担心各种奇怪的链接错误了。

理由五：RPP自由度极高，完全开源，您可以随意DIY运算符的优先级，增加新运算符，增加修改“内置类型”，甚至于int、double这些基础类型都是由用户自定义的，一切由您做主。

理由六：它与C++极为相似，C++程序员几乎无需学习即可使用，而且RPP所有数据类型和C++二进制兼容，无论是静态链接还是动态链接，想怎样就怎样。

##### RPP编码风格1：（类似《算法导论》的伪代码）

		define ← =  
		  
		void insertion_sort(rstr& a):
			for j ← 1 to a.count-1  
				key ← a[j]  
				i ← j-1  
				while i>=0 && a.get(i)>key  
					a[i+1] ← a[i]  
					i ← i-1  
				a[i+1] ← key

##### RPP编码风格2：（类似python的无花括号风格）

		bool next_permutation<T>(rbuf<T>& v):
			if v.count<=1  
				return false  
			next=v.count-1  
			for  
				temp=next  
				next--  
				if v[next]<v[temp]  
					mid=v.count-1  
					for !(v[next]<v[mid])  
						mid--  
					swap<T>(v[next],v[mid])  
					reverse<T>(v,temp)  
					return true  
				if next==0  
					reverse<T>(v,0)  
					return false

##### RPP编码风格3：（Lisp风格，从v1.91开始S表达式用逗号分隔）

		void main():
			int a
			int b
			[int,=,[a,1]]
			[int,=,[b,2]]
			[rf,print,[[int,+,[a,b]]]]
			
##### RPP编码风格4：（这是标准C++语法，本段代码可用VC++、G++或RPP进行编译）

		static rbool inherit_proc(tsh& sh,tclass& tci,int level=0)
		{
			if(level++>c_rpp_deep)
				return false;
			if(tci.vfather.empty())
				return true;
			rbuf<tword> v;
			for(int i=0;i<tci.vfather.count();i++)
			{
				rstr cname=tci.vfather[i].vword.get(0).val;
				tclass* ptci=zfind::class_search(sh,cname);
				if(ptci==null)
				{
					ptci=zfind::classtl_search(sh,cname);
					if(ptci==null)
						return false;
				}
				if(!inherit_proc(sh,*ptci,level))
					return false;
				v+=ptci->vword;
			}
			v+=tci.vword;
			tci.vword=v;
			return true;
		}

<br/>
RPP支持多种运行方式，方法如下：（各种运行方式支持的特性请参考rpp.pdf）

##### JIT：

1. cd到bin目录
2. 命令行敲入 rpp ..\src\example\test\1.rpp

##### 解释运行：

1. cd到bin目录
2. 命令行敲入 rpp -interpret ..\src\example\test\1.rpp

##### 编译运行（以NASM为后端）：

1. cd到bin目录
2. 命令行敲入 rnasm ..\src\example\test\1.rpp

##### 编译运行（翻译为C++并以G++为后端）：

1. cd到bin目录
2. 命令行敲入 gpp ..\src\example\test\1.rpp

##### HTML5在线解释运行（目前仅支持chrome）：

1. 点击 http://roundsheep.github.io/rpp_alpha/
2. 点击run按钮，稍等几秒会显示运行结果

##### IOS：

1. cd到bin目录
2. 命令行敲入 rpp -gpp ..\src\example\test\1.rpp
3. 将生成的src\example\test\1.cpp和ext\mingw\gpp.h两个文件导入xcode
4. 根据需要修改main函数，注释掉windows.h头文件

##### Android：（绿色集成包，不依赖其他环境）

1. 确保编译环境为64位windows
2. 下载一键安卓工具包并解压到一个无空格无中文的路径（1.1G） http://pan.baidu.com/s/1c0oc3Ws
3. 点击create_proj.bat
4. 输入工程名如test，等待命令窗口结束
5. 点击proj\test\build_cpp.bat
6. 等待几分钟命令窗口出现“请按任意键继续”
7. 按回车键并等待命令窗口结束
8. 成功后会得到proj\test\proj.android\bin\rpp.apk
9. 根据需要将RPP翻译得到的CPP文件包含进proj\test\Classes\HelloWorldScene.cpp

##### X64：

1. cd到bin目录
2. 命令行敲入 rpp -gpp64 ..\src\example\64bit_test.rpp
3. 将生成的src\example\test\64bit_test.cpp导入Visual Studio（或者使用64位的G++）
4. 选择x64
5. 按F7

<br/>
Visual Assist智能补全请看视频演示：

http://www.tudou.com/programs/view/40Ez3FuqE10/

<br/>
##### 重新将RPP编译为JS：

1. 确保安装了emscripten
2. 打开Emscripten Command Prompt
3. cd到RPP主目录
4. 命令行敲入 em++ -O3 src\rpp.cxx -o rpp.html -w --preload-file src -s TOTAL_MEMORY=156777216 -s EXPORTED_FUNCTIONS="['_js_main']"

##### 重新编译RPP源码：

1. 确保安装了 VS2012 update4 或者 VS2013
2. 打开src\proj\rpp.sln
3. 选择Release模式（因为JIT不支持Debug）
4. 按F7，成功后会生成bin\rpp.exe

##### 自举（以NASM为后端）：

1. 双击self_test_nasm.bat
2. 稍等几分钟后会生成bin\rpp_nasm.exe（实际上RPP完成自举只需要5秒，瓶颈在NASM，据说Chez Scheme自举也是5秒）
3. 注意自举后仅NASM模式和GPP模式可用

##### 自举（以G++为后端）：

1. 双击self_test_gpp.bat
2. 稍等几分钟后会生成bin\rpp_gpp.exe（使用命令gpp_build ..\src\rpp.cxx可13秒自举，只不过自举后第二次再自举需要60秒，-O0优化）
3. 注意自举后仅NASM模式和GPP模式可用

##### 调试：

1. cd到bin目录
2. 命令行敲入 rpp -gpp ..\src\example\test\1.rpp
3. 使用Visual Studio或gdb调试src\example\test\1.cpp

##### 自动测试example下所有例子：

1. 双击bin\example_test.bat

<br/>
RPP没有协议，任何人可以随意使用、复制、发布、修改、改名。


QQ交流群：34269848   

E-mail：287848066@qq.com
