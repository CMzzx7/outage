//index.js
//获取应用实例
const app = getApp()
const devicesId = "576919057"
const api_key = "p9fU76aB5ze2zDVK1ZHNmXgJuj0="

Page({
  data: {
    motto: 'Hello World',
    userInfo: {},
    hasUserInfo: false,
    canIUse: wx.canIUse('button.open-type.getUserInfo'),
    flag: "无人"
  },
  //事件处理函数
  bindViewTap: function () {
    wx.navigateTo({
      url: '../logs/logs'
    })
  },
  onLoad: function () {
    if (app.globalData.userInfo) {
      this.setData({
        userInfo: app.globalData.userInfo,
        hasUserInfo: true
      })
    } else if (this.data.canIUse) {
      // 由于 getUserInfo 是网络请求，可能会在 Page.onLoad 之后才返回
      // 所以此处加入 callback 以防止这种情况
      app.userInfoReadyCallback = res => {
        this.setData({
          userInfo: res.userInfo,
          hasUserInfo: true
        })
      }
    } else {
      // 在没有 open-type=getUserInfo 版本的兼容处理
      wx.getUserInfo({
        success: res => {
          app.globalData.userInfo = res.userInfo
          this.setData({
            userInfo: res.userInfo,
            hasUserInfo: true
          })
        }
      })
    }

    //每隔6s自动获取一次数据进行更新
    const timer = setInterval(() => {
      this.getDatapoints().then(datapoints => {
        this.update(datapoints)
      })
    }, 6000)
  },
  getUserInfo: function (e) {
    console.log(e)
    app.globalData.userInfo = e.detail.userInfo
    this.setData({
      userInfo: e.detail.userInfo,
      hasUserInfo: true
    })
  },
  getDatapoints: function () {
    return new Promise((resolve, reject) => {
      wx.request({
        url: `https://api.heclouds.com/devices/${devicesId}/datapoints?datastream_id=tixing&limit=20`,
        /**
         * 添加HTTP报文的请求头, 
         * 其中api-key为OneNet的api文档要求我们添加的鉴权秘钥
         * Content-Type的作用是标识请求体的格式, 从api文档中我们读到请求体是json格式的
         * 故content-type属性应设置为application/json
         */
        header: {
          'content-type': 'application/json',
          'api-key': api_key
        },
        success: (res) => {
          console.log(res)
          if (res.data.data.datastreams[0].datapoints[0].value == "1") {
            this.setData({
              flag: "有人"
            })

          } else {
            if (this.data.flag == "有人"){
              var that=this
              wx.showModal({
                content: "检测到用户已经离开，是否关闭电源？",
                showCancel: true,
                confirmText: '确定',
                success(res) {
                  if (res.confirm) {
                    console.log('用户点击了确定')
                    that.powerOff()
                  } else if (res.cancel) {
                    console.log('用户点击了取消')                    
                  }
                }
              })
            }
            this.setData({
              flag: "无人"
            })
          }
        },
        fail: (err) => {
          reject(err)
        }
      })
    })
  },
  update: function (datapoints) {
    const wheatherData = this.convert(datapoints);

    this.lineChart_hum.updateData({
      categories: wheatherData.categories,
      series: [{
        name: 'humidity',
        data: wheatherData.humidity,
        format: (val, name) => val.toFixed(2)
      }],
    })

    this.lineChart_light.updateData({
      categories: wheatherData.categories,
      series: [{
        name: 'light',
        data: wheatherData.light,
        format: (val, name) => val.toFixed(2)
      }],
    })

    this.lineChart_tempe.updateData({
      categories: wheatherData.categories,
      series: [{
        name: 'tempe',
        data: wheatherData.tempe,
        format: (val, name) => val.toFixed(2)
      }],
    })

  },
  powerOn: function () {
    wx.request({                                                //调用MQTT下发指令API
      url: `http://api.heclouds.com/cmds?device_id=${devicesId}`, //仅为示例，并非真实的接口地址
      data: "1",
      header: {
        'content-type': ' text/plain', // 默认值
        'api-key': api_key
      },
      method: 'POST',
      success(res) {
        console.log(res.data)
      }
    })
  },
  powerOff: function () {
    wx.request({                                                //调用MQTT下发指令API
      url: `http://api.heclouds.com/cmds?device_id=${devicesId}`, //仅为示例，并非真实的接口地址
      data: "0",
      header: {
        'content-type': ' text/plain', // 默认值
        'api-key': api_key
      },
      method: 'POST',
      success(res) {
        console.log(res.data)
      }
    })
  }
})
