#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

static void save_image(unsigned char *buf, int wrap, int xsize, int ysize, const char *filename, int is_color)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
    {
        fprintf(stderr, "Could not open %s\n", filename);
        return;
    }
    if (is_color)
    {
        fprintf(f, "P6\n%d %d\n255\n", xsize, ysize);
    }
    else
    {
        fprintf(f, "P5\n%d %d\n255\n", xsize, ysize);
    }
    for (int i = 0; i < ysize; i++)
    {
        fwrite(buf + i * wrap, 1, xsize * (is_color ? 3 : 1), f);
    }
    fclose(f);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    char **filenames = user_data;
    GtkWidget *window, *image;
    for (int i = 0; i < 3; i++)
    {
        window = gtk_application_window_new(app);
        gtk_window_set_title(GTK_WINDOW(window), i == 0 ? "Colored Image" : "Grayscale Image");
        gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
        image = gtk_image_new_from_file(filenames[i]);
        gtk_window_set_child(GTK_WINDOW(window), image);
        gtk_window_present(GTK_WINDOW(window));
    }
}

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        fprintf(stderr, "Usage: %s <input video> <frame number> <R> <G> <B>\n", argv[0]);
        exit(1);
    }

    avformat_network_init();
    AVFormatContext *fmt_ctx = NULL;
    if (avformat_open_input(&fmt_ctx, argv[1], NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return -1;
    }
    avformat_find_stream_info(fmt_ctx, NULL);

    int video_stream_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1)
    {
        fprintf(stderr, "No video stream found in %s\n", argv[1]);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);
    const AVCodec *codec = avcodec_find_decoder(codec_ctx->codec_id);
    avcodec_open2(codec_ctx, codec, NULL);

    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    struct SwsContext *sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                                                codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
                                                SWS_BICUBIC, NULL, NULL, NULL);

    int frame_count = 0;
    while (av_read_frame(fmt_ctx, &packet) >= 0 && frame_count <= atoi(argv[2]))
    {
        if (packet.stream_index == video_stream_index)
        {
            avcodec_send_packet(codec_ctx, &packet);
            if (avcodec_receive_frame(codec_ctx, frame) == 0 && frame_count++ == atoi(argv[2]))
            {
                int rgb_size = codec_ctx->width * codec_ctx->height * 3;
                uint8_t *rgb_buffer = (uint8_t *)av_malloc(rgb_size);
                uint8_t *rgb_data[1] = {rgb_buffer};
                int rgb_linesize[1] = {3 * codec_ctx->width};
                sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
                          codec_ctx->height, rgb_data, rgb_linesize);
                save_image(rgb_buffer, rgb_linesize[0], codec_ctx->width, codec_ctx->height, "frame.ppm", 1);

                uint8_t *gray_buffer = (uint8_t *)av_malloc(codec_ctx->width * codec_ctx->height);
                for (int i = 0; i < codec_ctx->width * codec_ctx->height; i++)
                {
                    int r = rgb_buffer[i * 3];
                    int g = rgb_buffer[i * 3 + 1];
                    int b = rgb_buffer[i * 3 + 2];
                    gray_buffer[i] = (uint8_t)(r * atof(argv[3]) + g * atof(argv[4]) + b * atof(argv[5]));
                }
                save_image(gray_buffer, codec_ctx->width, codec_ctx->width, codec_ctx->height, "frame.pgm", 0);

                av_free(rgb_buffer);
                av_free(gray_buffer);
                break;
            }
        }
        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
    sws_freeContext(sws_ctx);

    GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    char *filenames[] = {"frame.ppm", "frame.pgm"};
    g_signal_connect(app, "activate", G_CALLBACK(activate), filenames);
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}