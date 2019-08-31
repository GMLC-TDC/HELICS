function v = helics_core_type_tcp()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 6);
  end
  v = vInitialized;
end
