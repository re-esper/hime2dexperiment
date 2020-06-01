# Hime 2D Engine Experiments

Quick-and-dirty demos to verify some ideas and techniques of my hime 2d game engine.

## Hime 2D BunnyMark

分别用hime2d实验代码的C++与Lua接口实现标准bunnymark, 并和cocos2d-x的C++与Lua版本的bunnymark比较性能

主要比较**非渲染**部分的性能

另请参见 [BunnyMarkGame](https://github.com/re-esper/BunnyMarkGame)

## 说明

对于him2d版本, 以下宏定义位于[hime2d.h](https://github.com/re-esper/hime2dexperiment/blob/master/hime2dbunnymark/hime2d/hime2d.h):  
宏定义``HIME2D_LUA`` 切换使用Lua逻辑或C++逻辑  
宏定义``NORENDER`` 切换是否关闭精灵渲染

对于cocos2d-x版本, 测试时关闭渲染的方式是注释掉``CCSprite.cpp``中``Sprite::draw``的全部函数体

与[BunnyMarkGame](https://github.com/re-esper/BunnyMarkGame)不同的是, hime2d实验代码的图形管线和cocos2d-x完全一致, shader完全一致, 也基本实现了相同的2D精灵功能, 部分缺失的功能不会对性能产生明显影响

### 测试环境

Intel Core i7-7700K + Intel HD 630  
渲染部分在**独显**或者**安卓**上比起cocos2d-x会有更大优势, 然而我的显卡坏了- -||

## 结果

|        | Lua无渲染 | Lua  | C++无渲染 | C++ |
| ------------ | ------------ | ------------ | ------------ | ------------ |
| hime2d   | **370,000**     | **170,000**     | **400,000**     | **170,000**     |
| cocos2d-x    | 22,000     | 15,000    | 57,000     | 18,500     |
| **Ratio**    | **16.8x**     | **11.3x**    | **7.0x**     | **9.2x**     |

<img src="https://github.com/re-esper/hime2dexperiment/blob/master/pics/hime2dlua.png" width="50%" height="50%"> 
