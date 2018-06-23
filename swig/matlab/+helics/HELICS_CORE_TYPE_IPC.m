function v = HELICS_CORE_TYPE_IPC()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876590);
  end
  v = vInitialized;
end
