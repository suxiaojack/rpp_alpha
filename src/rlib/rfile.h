﻿#pragma once

#include "rstrw.h"

struct rfile
{
	void* m_fp;
	
	~rfile()
	{
		close();
	}

	rfile()
	{
		m_fp=null;
	}

	rfile(rstrw name,rstrw mode=rstr("r"))//todo: 所有文件名改为rstr
	{
		open(name,mode);
	}

	rbool close()
	{
		if(null==m_fp)
		{
			return false;
		}
		if(xf::fclose(m_fp)!=0)
		{
			return false;
		}
		m_fp=null;
		return true;
	}
	
	//mode: rb只读，wb重新创建写,rb+读写
	//推荐用 r只读，w写，rw读写，本函数只有二进制模式
	rbool open(rstrw name,rstrw mode=rstr("r"))
	{
		m_fp=null;
		if(!exist(name))
		{
			if(rstrw("rw")==mode)
			{
				if(!create(name))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		if(rstrw("r")==mode)
		{
			mode=rstrw("rb");
		}
		elif(rstrw("w")==mode)
		{
			mode=rstrw("wb");
		}
		elif(rstrw("rw")==mode)
		{
			mode=rstrw("rb+");
		}
		m_fp=xf::fopenw(name.cstrw_t(),mode.cstrw_t());
		if(null==m_fp)
		{
			return false;
		}
		return true;
	}

	//强制打开一个新文件
	rbool open_n(rstrw name,rstrw mode=rstr("rw"))
	{
		xf::remove(name.cstrw_t());
		return open(name,mode);
	}

	rbool read(int off,int len,void* buf)
	{
		return read((int8)off,len,buf);
	}

	rbool read(int8 off,int len,void* buf)
	{
		if(null==m_fp)
		{
			return false;
		}
		set_off8(off);
		return len==(int)(xf::fread(buf,1,len,m_fp));
	}

	rbool read(int len,void* buf)
	{
		if(null==m_fp)
		{
			return false;
		}
		return len==(int)(xf::fread(buf,1,len,m_fp));
	}
	
	//读取并返回整个文件，通常用于小文件
	rstr read_all()
	{
		rstr ret;
		ret.set_size(size());
		if(!read(0,size(),ret.begin()))
		{
			return rstr();
		}
		return r_move(ret);
	}

	//直接读一个小文件
	static rstr read_all_n(const rstrw& name)
	{
		rfile file(name);
		rstr ret;
		ret.set_size(file.size());
		if(!file.read(0,file.size(),ret.begin()))
		{
			return rstr();
		}
		return r_move(ret);
	}
	
	//读取一行（跳过空行）
	rstr read_line()
	{
		rstr ret;
		uchar c;
		while(read(1,&c))
		{
			if(r_char('\r')==c||r_char('\n')==c)
			{
				if(0==ret.count())
				{
					continue;
				}
				else
				{
					return ret;
				}
			}
			ret+=c;
		}
		return r_move(ret);
	}
	
	//读取一行，仅用于utf16文件
	rstrw read_line_w()
	{
		rstrw ret;
		ushort c;
		while(read(2,&c))
		{
			if(r_char('\r')==c||r_char('\n')==c)
			{
				if(0==ret.count())
				{
					continue;
				}
				else
				{
					return ret;
				}
			}
			ret+=c;
		}
		return r_move(ret);
	}

	rbool write(int off,int len,const void* buf)
	{
		return write((int8)off,len,buf);
	}

	rbool write(int8 off,int len,const void* buf)
	{
		if(null==m_fp)
		{
			return false;
		}
		set_off8(off);
		return len==(int)(xf::fwrite(buf,1,len,m_fp));
	}

	rbool write(int len,const void* buf)
	{
		if(null==m_fp)
		{
			return false;
		}
		return len==(int)(xf::fwrite(buf,1,len,m_fp));
	}

	rbool write(rstr s)
	{
		return write(s.size(),s.begin());
	}

	rbool write_line(rstr s)
	{
		return write(s.size(),s.begin())&&write(1,"\n");
	}

	rbool write_line_w(rstrw s)
	{
		return write(s.size(),s.begin())&&write(2,rstrw("\n").cstrw_t());
	}

	static rbool write_all_n(const rstrw& name,const rstr& cont)
	{
		rfile file;
		ifn(file.open_n(name))
		{
			return false;
		}
		return file.write(cont);
	}

	int size()
	{
		return (int)(size8());
	}

	int8 size8()
	{
		if(null==m_fp)
		{
			return 0;//返回-1和返回0各有利弊
		}
		int8 cur=get_off8();
		xf::fseek8(m_fp,0,xf::X_SEEK_END);
		int8 ret=get_off8();
		set_off8(cur);
		return ret;
	}

	int get_off()
	{
		return (int)(get_off8());
	}

	int8 get_off8()
	{
		if(null==m_fp)
		{
			return 0;
		}
		return xf::ftell8(m_fp);
	}

	rbool set_off(int off)
	{
		return set_off8((int8)off);
	}

	rbool set_off8(int8 off)
	{
		return 0==xf::fseek8(m_fp,off,xf::X_SEEK_SET);
	}

	static rbool exist(rstrw name)
	{
		void* fp=xf::fopenw(name.cstrw_t(),rstrw("rb").cstrw_t());
		if(null==fp)
		{
			return false;
		}
		xf::fclose(fp);
		return true;
	}

	//强制创建文件
	static rbool create(rstrw name,int8 size=0)
	{
		void* fp=xf::fopenw(name.cstrw_t(),rstrw("wb").cstrw_t());
		if(null==fp)
		{
			return false;
		}
		xf::fclose(fp);
		if(size>0)
		{
			rfile file(name);
			file.set_off8(size-1);
			if(!file.write(1,"\0"))
			{
				return false;
			}
			return file.close();
		}
		return true;
	}

	static int8 get_size8(rstrw name)
	{
		rfile file(name);
		return file.size8();
	}

	static int remove(rstrw name)
	{
		return xf::remove(name.cstrw_t());
	}
};
