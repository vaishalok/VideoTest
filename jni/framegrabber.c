
typedef struct VideoState {
	AVFormatContext *pFormatCtx;
	AVStream *pVideoStream;
	int videoStreamIdx;
}

VideoState;
VideoState *gvs;


int naInit(JNIEnv *pEnv, jobject pObj, jstring pfilename) {
	gVideoFileName = (char *)(*pEnv)->GetStringUTFChars(pEnv, pfilename, NULL);
	av_register_all();
	VideoState *vs;
	vs = av_mallocz(sizeof(VideoState));
	gvs = vs;
	av_register_all();
	//open the video file
	avformat_open_input(&vs->pFormatCtx, gVideoFileName, NULL, NULL);
	//retrieve stream info
	avformat_find_stream_info(vs->pFormatCtx, NULL);

	//find the video stream
	int i;
	AVCodecContext *pcodecctx;
	//find the first video stream
	vs->videoStreamIdx = -1;
	for (i = 0; i < vs->pFormatCtx->nb_streams; ++i) {
		if (AVMEDIA_TYPE_VIDEO == vs->pFormatCtx->streams[i]->codec->codec_type) {
			vs->videoStreamIdx = i;
			vs->pVideoStream = vs->pFormatCtx->streams[i];
			break;
		}
	}
	//get the decoder from the video stream
	pcodecctx = vs->pFormatCtx->streams[vs->videoStreamIdx]->codec;
	AVCodec *pcodec;
	pcodec = avcodec_find_decoder(pcodecctx->codec_id);
	//open the codec
	avcodec_open2(pcodecctx, pcodec, NULL);
	return 0;
}


jintArray naGetVideoRes(JNIEnv *pEnv, jobject pObj) {
	jintArray lRes;
	AVCodecContext* vCodecCtx = gvs->pVideoStream->codec;
	lRes = (*pEnv)->NewIntArray(pEnv, 2);
	jint lVideoRes[2];
	lVideoRes[0] = vCodecCtx->width;
	lVideoRes[1] = vCodecCtx->height;
	(*pEnv)->SetIntArrayRegion(pEnv, lRes, 0, 2, lVideoRes);
	return lRes;
}


jint naGetDuration(JNIEnv *pEnv, jobject pObj) {
	return (gvs->pFormatCtx->duration / AV_TIME_BASE);
}

jint naGetFrameRate(JNIEnv *pEnv, jobject pObj) {
	int fr;
	VideoState *vs = gvs;
	if(vs->pVideoStream->avg_frame_rate.den && vs->pVideoStream->avg_frame_rate.num) {
		fr = av_q2d(vs->pVideoStream->avg_frame_rate);
	} else if(vs->pVideoStream->r_frame_rate.den && vs->pVideoStream->r_frame_rate.num) {
		fr = av_q2d(vs->pVideoStream->r_frame_rate);
	} else if(vs->pVideoStream->time_base.den && vs->pVideoStream->time_base.num) {
		fr = 1/av_q2d(vs->pVideoStream->time_base);
	} else if(vs->pVideoStream->codec->time_base.den && vs->pVideoStream->codec->time_base.num) {
		fr = 1/av_q2d(vs->pVideoStream->codec->time_base);
	}
	return fr;
}

int naFinish(JNIEnv *pEnv, jobject pObj) {
	VideoState* vs = gvs;
	//close codec
	avcodec_close(vs->pVideoStream->codec);
	//close video file
	avformat_close_input(&vs->pFormatCtx);
	av_free(vs);
	return 0;
}


