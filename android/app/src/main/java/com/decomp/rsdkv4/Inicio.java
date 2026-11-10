package com.decomp.rsdkv4;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class Inicio extends Activity {

    private static final String PASTA_JOGO = "Sonic1SIR";

    private TextView titulo;
    private TextView texto;
    private Button botao1;
    private Button botao2;
    private int modo = 0; // 0 = falta acesso aos arquivos, 1 = falta o Data.rsdk
    private boolean pediuAcesso = false;

    private int dp(int v) {
        return (int) (v * getResources().getDisplayMetrics().density);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        int pad = dp(24);
        LinearLayout raiz = new LinearLayout(this);
        raiz.setOrientation(LinearLayout.VERTICAL);
        raiz.setGravity(Gravity.CENTER);
        raiz.setPadding(pad, pad, pad, pad);

        titulo = new TextView(this);
        titulo.setTextColor(Color.WHITE);
        titulo.setTextSize(28);
        titulo.setTypeface(Typeface.DEFAULT_BOLD);
        titulo.setGravity(Gravity.CENTER);

        texto = new TextView(this);
        texto.setTextColor(Color.WHITE);
        texto.setTextSize(18);
        texto.setGravity(Gravity.CENTER);
        texto.setPadding(0, dp(20), 0, dp(24));

        botao1 = new Button(this);
        botao1.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (modo == 0) {
                    pedirAcesso();
                } else {
                    atualizar(true);
                }
            }
        });

        botao2 = new Button(this);
        botao2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                finish();
            }
        });

        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.setMargins(0, dp(6), 0, dp(6));

        raiz.addView(titulo, lp);
        raiz.addView(texto, lp);
        raiz.addView(botao1, lp);
        raiz.addView(botao2, lp);

        ScrollView sv = new ScrollView(this);
        sv.setFillViewport(true);
        sv.setBackgroundColor(Color.parseColor("#0B1A6B"));
        sv.addView(raiz);
        setContentView(sv);
    }

    @Override
    protected void onResume() {
        super.onResume();
        atualizar(false);
    }

    private String pastaBase() {
        return Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + PASTA_JOGO;
    }

    private boolean temAcesso() {
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

    private void pedirAcesso() {
        if (Build.VERSION.SDK_INT >= 30) {
            try {
                Intent i = new Intent("android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION",
                        Uri.parse("package:" + getPackageName()));
                startActivity(i);
            } catch (Exception e) {
                try {
                    startActivity(new Intent("android.settings.MANAGE_ALL_FILES_ACCESS_PERMISSION"));
                } catch (Exception e2) {
                    Toast.makeText(this, "Abra as configurações do app e ative o acesso a todos os arquivos",
                            Toast.LENGTH_LONG).show();
                }
            }
        } else {
            requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
        }
    }

    private void criarPastas() {
        String base = pastaBase();
        new File(base).mkdirs();
        new File(base + "/mods").mkdirs();
    }

    private void mostrar(String t, String msg, String b1, String b2) {
        titulo.setText(t);
        texto.setText(msg);
        botao1.setText(b1);
        botao2.setText(b2);
    }

    private void atualizar(boolean manual) {
        if (!temAcesso()) {
            modo = 0;
            mostrar("Sonic 1 S.I.R",
                    "O jogo precisa de acesso aos arquivos para criar a pasta Sonic1SIR e ler o Data.rsdk.\n\n"
                    + "Toque em \"Conceder acesso\" e ative a permissão.\n\n"
                    + "The game needs file access to create the Sonic1SIR folder and read Data.rsdk.",
                    "Conceder acesso", "Fechar");
            if (!pediuAcesso) {
                pediuAcesso = true;
                pedirAcesso();
            }
            return;
        }

        criarPastas();

        if (!new File(pastaBase() + "/Data.rsdk").exists()) {
            modo = 1;
            mostrar("Sonic 1 S.I.R",
                    "Coloque o Data.rsdk de Sonic 1 na pasta Sonic1SIR para o jogo rodar!\n\n"
                    + "Pasta: " + pastaBase() + "/\n\n"
                    + "Put the Sonic 1 Data.rsdk file in the Sonic1SIR folder to run the game!",
                    "Já coloquei", "Fechar");
            if (manual) {
                Toast.makeText(this, "Data.rsdk ainda não encontrado na pasta Sonic1SIR",
                        Toast.LENGTH_LONG).show();
            }
            return;
        }

        startActivity(new Intent(this, RSDKv4.class));
        finish();
    }
}
