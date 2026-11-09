package com.decomp.rsdkv4;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.widget.Toast;

import java.io.File;

import org.libsdl.app.*;

public class RSDKv4 extends SDLActivity {

    private static final String PASTA_JOGO = "Sonic1SIR";

    private boolean aguardandoAcesso = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (!temAcessoArmazenamento()) {
            aguardandoAcesso = true;
            pedirAcessoArmazenamento();
        }
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onStart() {
        super.onStart();
        criarPastas();
    }

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

        // Voltou da tela de permissao: se foi concedida, reinicia o app
        // para o motor reler o Data.rsdk na pasta Sonic1SIR
        if (aguardandoAcesso && temAcessoArmazenamento()) {
            aguardandoAcesso = false;
            criarPastas();
            reiniciarApp();
        }
    }

    private boolean temAcessoArmazenamento() {
        if (Build.VERSION.SDK_INT >= 30) {
            try {
                Object r = Environment.class.getMethod("isExternalStorageManager").invoke(null);
                return (r instanceof Boolean) && ((Boolean) r).booleanValue();
            } catch (Exception e) {
                return false;
            }
        }
        return checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED;
    }

    private void pedirAcessoArmazenamento() {
        Toast.makeText(this,
                "Sonic 1 S.I.R precisa de acesso aos arquivos para ler o Data.rsdk na pasta Sonic1SIR",
                Toast.LENGTH_LONG).show();
        if (Build.VERSION.SDK_INT >= 30) {
            try {
                Intent i = new Intent("android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION",
                        Uri.parse("package:" + getPackageName()));
                startActivity(i);
            } catch (Exception e) {
                try {
                    startActivity(new Intent("android.settings.MANAGE_ALL_FILES_ACCESS_PERMISSION"));
                } catch (Exception e2) {
                    // sem tela de permissao disponivel
                }
            }
        } else {
            requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
        }
    }

    private void criarPastas() {
        if (!temAcessoArmazenamento()) {
            return;
        }
        String base = Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + PASTA_JOGO;
        new File(base).mkdirs();
        new File(base + "/mods").mkdirs();
    }

    private void reiniciarApp() {
        Intent i = getPackageManager().getLaunchIntentForPackage(getPackageName());
        if (i != null) {
            i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            startActivity(i);
        }
        finishAffinity();
        android.os.Process.killProcess(android.os.Process.myPid());
    }

    // Chamado pelo motor (C++) via JNI: devolve a pasta do jogo
    public String getBasePath() {
        String p = Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + PASTA_JOGO;
        if (temAcessoArmazenamento()) {
            new File(p).mkdirs();
            new File(p + "/mods").mkdirs();
        }
        return p + "/";
    }
}
