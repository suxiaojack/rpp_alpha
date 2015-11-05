#pragma once

#include "ybase.h"

//词法分析
struct yword
{
	static rbool analyse(const tsh& sh,rstr& src,rbuf<tword>& result,const tfile* pfile)
	{
		result.clear();
		tword cur_word;
		cur_word.pos.file=(tfile*)pfile;
		cur_word.pos.line=1;
		int len;
		uchar* start;
		uchar* p;
		rstr stemp;
		stemp.set_size(sh.m_optr.m_optr_max);
		for(p=src.cstr();*p!=0;++p)
		{
			start=p;
			if(*p==r_char('/'))
			{
				//为了效率这里使用了过多的缩进
				if(*(p+1)==r_char('*'))
				{
					p+=2;
					int count=1;
					for(;*p!=0&&*(p+1)!=0;p++)
					{
						if(r_char('\n')==*p)
						{
							++cur_word.pos.line;
						}
						if(*p!=0&&*(p+1)!=0&&
							*p==r_char('/')&&
							*(p+1)==r_char('*'))
						{
							++count;
						}
						if(*p!=0&&*(p+1)!=0&&
							*p==r_char('*')&&
							*(p+1)==r_char('/'))
						{
							--count;
						}
						if(0==count)
						{
							break;
						}
					}
					if(count)
					{
						rpperror(rstr(cur_word.pos.line)+
							rstr("miss */"));
						return false;
					}
					p++;
					continue;
				}
				elif(*(p+1)==r_char('/'))
				{
					p+=2;
					for(;*p&&*p!=r_char('\n');p++)
					{
						;
					}
					if(*p==0)
					{
						return true;
					}
					++cur_word.pos.line;
					continue;
				}
			}
			len=get_optr_s_len(sh,p,src.end()-p,stemp);
			if(len)
			{
				cur_word.val=rstr(p,p+len);
				push_word(result,cur_word);
				p+=len;
				p--;
			}
			elif(rstr::is_alpha(*p)||r_char('_')==*p||
				rcode::is_utf8_3(*p)&&*(p+1)!=0&&*(p+2)!=0||
				rcode::is_utf8_2(*p)&&*(p+1)!=0)
			{
				for(;*p!=0;++p)
				{
					if(rstr::is_number(*p)||
						rstr::is_alpha(*p)||r_char('_')==*p)
					{
						continue;
					}
					elif(rcode::is_utf8_3(*p)&&*(p+1)!=0&&*(p+2)!=0)
					{
						//todo
						//应该只能以汉字开头不能以特殊符号开头
						p+=2;
						continue;
					}
					elif(rcode::is_utf8_2(*p)&&*(p+1)!=0)
					{
						p++;
						continue;
					}
					else
					{
						break;
					}
				}
				cur_word.val=rstr(start,p);
				push_word(result,cur_word);
				p--;
			}
			elif(rstr::is_number(*p))
			{
				for(++p;*p!=0&&(rstr::is_number(*p)||r_char('_')==*p||
					rstr::is_alpha(*p));++p)
				{
					;
				}
				cur_word.val=rstr(start,p);
				push_word(result,cur_word);
				p--;
			}
			elif(r_char('\"')==*p)
			{
				int line=cur_word.pos.line;
				for(++p;*p!=0;++p)
				{
					if(r_char('\n')==*p)
					{
						++cur_word.pos.line;
					}
					if(*p==r_char('\\'))
					{
						if(*(p+1)==r_char('x')&&*(p+2)!=0&&*(p+3)!=0)
						{
							p+=3;
						}
						elif(*(p+1)!=0)
						{
							p++;
						}
						continue;
					}
					if(*p==r_char('\"'))
					{
						break;
					}
				}
				cur_word.pos.line=line;
				if(*p!=0)
				{
					cur_word.val=rstr(start,p+1);
					push_word(result,cur_word);
				}
				else
				{
					rpperror(rstr(cur_word.pos.line)+
						rstr("miss \""));
					return false;
				}
			}
			elif(r_char('\'')==*p)
			{
				int line=cur_word.pos.line;
				for(++p;*p!=0;++p)
				{
					if(r_char('\n')==*p)
					{
						++cur_word.pos.line;
					}
					if(*p==r_char('\\'))
					{
						if(*(p+1)==r_char('x')&&*(p+2)!=0&&*(p+3)!=0)
						{
							p+=3;
						}
						elif(*(p+1)!=0)
						{
							p++;
						}
						continue;
					}
					if(*p==r_char('\''))
					{
						break;
					}
				}
				cur_word.pos.line=line;
				if(*p!=0)
				{
					cur_word.val=rstr(start,p+1);
					if(cur_word.val.count()<2)
					{
						rpperror(rstr(cur_word.pos.line)+
							rstr("miss '"));
						return false;
					}
					cur_word.val[0]=r_char('\"');
					cur_word.val[cur_word.val.count()-1]=r_char('\"');
					add_rstr(sh,result,cur_word);
				}
				else
				{
					rpperror(rstr(cur_word.pos.line)+
						rstr("miss '"));
					return false;
				}
			}
			elif(r_char('\n')==*p)
			{
				++cur_word.pos.line;
			}
			elif(r_char('\\')==*p&&r_char('\\')==*(p+1))
			{
				p+=2;
				for(;*p!=0&&*p!=r_char('\n')&&*p!=0xd;p++)
				{
					;
				}
				cur_word.val=ybase::add_quote(rstr(start+2,p));
				add_rstr(sh,result,cur_word);
				if(*p==0)
				{
					return true;
				}
				if(*p==r_char('\n'))
				{
					++cur_word.pos.line;
				}
			}
			elif(r_char('`')==*p)
			{
				if(*(p+1)==0)
				{
					rpperror(rstr(cur_word.pos.line)+
						rstr("miss `"));
					return false;
				}
				p++;
				cur_word.val=rstr((uint)(*(p)));
				push_word(result,cur_word);
			}
		}
		return true;
	}

	static void push_word(rbuf<tword>& result,tword word)
	{
		word.pos_src=word.pos;
		result.push_move(word);
	}

	static int get_optr_s_len(const tsh& sh,const uchar* s,int len,rstr& stemp)
	{
		for(int i=sh.m_optr.m_optr_max;i>0;i--)
		{
			if(len<i)
			{
				continue;
			}
			xf::memcpy(stemp.begin(),s,i);
			stemp.m_buf.m_count=i;
			int pos=r_find_b<rstr>(sh.m_optr.m_optr_s,stemp);//这里是瓶颈
			if(pos<sh.m_optr.m_optr_s.count())
			{
				return sh.m_optr.m_optr_s[pos].count();
			}
		}
		return 0;
	}

	static void add_rstr(const tsh& sh,rbuf<tword>& result,const tword& word)
	{
		if(result.get_top()==rppkey(c_import)||
			result.get_top()==rppkey(c_include))
		{
			push_word(result,word);
			return;
		}
		tword temp;
		temp.pos=word.pos;
		temp.val=rppkey(c_rstr);
		push_word(result,temp);
		temp.val=rppoptr(c_sbk_l);
		push_word(result,temp);
		push_word(result,word);
		temp.val=rppoptr(c_sbk_r);
		push_word(result,temp);
	}
};
