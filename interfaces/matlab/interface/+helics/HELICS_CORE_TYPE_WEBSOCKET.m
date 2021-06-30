function v = HELICS_CORE_TYPE_WEBSOCKET()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 12);
  end
  v = vInitialized;
end
