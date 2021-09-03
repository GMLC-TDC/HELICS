function v = helics_flag_use_json_serialization()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 42);
  end
  v = vInitialized;
end
