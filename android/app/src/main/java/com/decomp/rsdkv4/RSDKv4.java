package com.decomp.rsdkv4;

import android.os.Environment;

import java.io.File;

import org.libsdl.app.*;

public class RSDKv4 extends SDLActivity {

    private static final String PASTA_JOGO = "Sonic1SIR";

    // Pausa/retoma a thread nativa quando nao ha multi-janela
    @Override
    protected void onPause() {
        super.onPause();
        if (mHasMultiWindow) {
            pauseNativeThread();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (mHasMultiWindow) {
            resumeNativeThread();
        }
    }

    // Chamado pelo motor (C++) via JNI: devolve a pasta do jogo
    public String getBasePath() {
        String p = Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + PASTA_JOGO;
        new File(p).mkdirs();
        new File(p + "/mods").mkdirs();
        return p + "/";
    }
}
