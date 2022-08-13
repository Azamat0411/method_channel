package com.example.method_channel;

import android.app.Activity;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.embedding.engine.plugins.activity.ActivityAware;
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.PluginRegistry.Registrar;

/**
 * PrintingPlugin
 */
public class PrintingPlugin implements FlutterPlugin, ActivityAware {
    private Activity activity;
    private MethodChannel channel;
    private PrintingHandler handler;

    /**
     * Plugin registration.
     * for legacy Embedding API
     */
    public static void registerWith(Registrar registrar) {
        Activity activity = registrar.activity();
        if (activity == null) {
            return; // We can't print without an activity
        }

        final PrintingPlugin instance = new PrintingPlugin();
        instance.onAttachedToEngine(registrar.messenger());
        instance.onAttachedToActivity(activity);
    }

    @Override
    public void onAttachedToEngine(FlutterPluginBinding binding) {
        onAttachedToEngine(binding.getBinaryMessenger());
    }

    private void onAttachedToEngine(BinaryMessenger messenger) {
        channel = new MethodChannel(messenger, "net.nfet.printing");

        if (activity != null) {
            handler = new PrintingHandler(activity, channel);
            channel.setMethodCallHandler(handler);
        }
    }

    @Override
    public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {
        channel.setMethodCallHandler(null);
        channel = null;
        handler = null;
    }

    @Override
    public void onAttachedToActivity(ActivityPluginBinding binding) {
        onAttachedToActivity(binding.getActivity());
    }

    private void onAttachedToActivity(Activity _activity) {
        activity = _activity;

        if (activity != null && channel != null) {
            handler = new PrintingHandler(activity, channel);
            channel.setMethodCallHandler(handler);
        }
    }

    @Override
    public void onDetachedFromActivityForConfigChanges() {
        onDetachedFromActivity();
    }

    @Override
    public void onReattachedToActivityForConfigChanges(ActivityPluginBinding binding) {
        onAttachedToActivity(binding.getActivity());
    }

    @Override
    public void onDetachedFromActivity() {
        channel.setMethodCallHandler(null);
        activity = null;
        handler = null;
    }
}
