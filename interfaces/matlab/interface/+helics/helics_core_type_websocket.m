function v = helics_core_type_websocket()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 12);
  end
  v = vInitialized;
end
