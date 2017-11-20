#ifndef PLOYFIT
#define PLOYFIT

#include <iostream>
#include <stdio.h>
//#include <conio.h>
#include <stdlib.h>
#include <math.h>

//计算多项式拟合
//http://blog.163.com/xing_mu_1/blog/static/661429020097101037114/
//经测试，直线（一项式）也是可以计算的的
/*==================polyfit(n,x,y,poly_n,a)===================*/
/*=======拟合y=a0+a1*x+a2*x^2+……+apoly_n*x^poly_n========*/
/*=====n是数据个数  xy是数据值  poly_n是多项式的项数======*/
/*===返回a0,a1,a2,……a[poly_n]，系数比项数多一（常数项）=====*/
extern void polyfit(int n,double x[],double y[],int poly_n,double a[]);

extern void gauss_solve(int n,double A[],double x[],double b[]);

#endif // PLOYFIT

