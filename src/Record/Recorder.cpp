﻿/*
 * Copyright (c) 2016 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/xiongziliang/ZLMediaKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include "Recorder.h"
#include "Common/config.h"
#include "Common/MediaSource.h"
#include "MP4Recorder.h"
#include "HlsRecorder.h"

using namespace toolkit;

namespace mediakit {

string Recorder::getRecordPath(Recorder::type type, const string &vhost, const string &app, const string &stream_id, const string &customized_path) {
    GET_CONFIG(bool, enableVhost, General::kEnableVhost);
    switch (type) {
        case Recorder::type_hls: {
            GET_CONFIG(string, hlsPath, Hls::kFilePath);
            string m3u8FilePath;
            if (enableVhost) {
                m3u8FilePath = vhost + "/" + app + "/" + stream_id + "/hls.m3u8";
            } else {
                m3u8FilePath = app + "/" + stream_id + "/hls.m3u8";
            }
            //Here we use the customized file path.
            if (!customized_path.empty()) {
                m3u8FilePath = customized_path + "/hls.m3u8";
            }
            return File::absolutePath(m3u8FilePath, hlsPath);
        }
        case Recorder::type_mp4: {
            GET_CONFIG(string, recordPath, Record::kFilePath);
            GET_CONFIG(string, recordAppName, Record::kAppName);
            string mp4FilePath;
            if (enableVhost) {
                mp4FilePath = vhost + "/" + recordAppName + "/" + app + "/" + stream_id + "/";
            } else {
                mp4FilePath = recordAppName + "/" + app + "/" + stream_id + "/";
            }
            //Here we use the customized file path.
            if (!customized_path.empty()) {
                /*开始删除*/
                // mp4FilePath = customized_path + "/";
                /*删除结束*/
                /*开始添加*/
				//@子悦，你上次说这里为了安全不能跳出目录，但实际操作过程中因为存储挂载位置不由流媒体决定，为了方便保存到挂载存储上，我这边做成可以跳出自已目录，你看是否合适，或者有其他办法可以处理这事
                return  customized_path + "/"+mp4FilePath;
                /*开始添加*/
            }

            return File::absolutePath(mp4FilePath, recordPath);
        }
        default:
            return "";
    }
}

std::shared_ptr<MediaSinkInterface> Recorder::createRecorder(type type, const string &vhost, const string &app, const string &stream_id, const string &customized_path){
    auto path = Recorder::getRecordPath(type, vhost, app, stream_id, customized_path);
    switch (type) {
        case Recorder::type_hls: {
#if defined(ENABLE_HLS)
            auto ret = std::make_shared<HlsRecorder>(path, string(VHOST_KEY) + "=" + vhost);
            ret->setMediaSource(vhost, app, stream_id);
            return ret;
#endif
            return nullptr;
        }

        case Recorder::type_mp4: {
#if defined(ENABLE_MP4)
            return std::make_shared<MP4Recorder>(path, vhost, app, stream_id);
#endif
            return nullptr;
        }

        default:
            return nullptr;
    }
}

bool Recorder::isRecording(type type, const string &vhost, const string &app, const string &stream_id){
    auto src = MediaSource::find(vhost, app, stream_id);
    if(!src){
        return false;
    }
    return src->isRecording(type);
}

bool Recorder::startRecord(type type, const string &vhost, const string &app, const string &stream_id,const string &customized_path){
    auto src = MediaSource::find(vhost, app, stream_id);
    if (!src) {
        WarnL << "未找到相关的MediaSource,startRecord失败:" << vhost << "/" << app << "/" << stream_id;
        return false;
    }
    return src->setupRecord(type, true, customized_path);
}

bool Recorder::stopRecord(type type, const string &vhost, const string &app, const string &stream_id){
    auto src = MediaSource::find(vhost, app, stream_id);
    if(!src){
        return false;
    }
    return src->setupRecord(type, false, "");
}

} /* namespace mediakit */
