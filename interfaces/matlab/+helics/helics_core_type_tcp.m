function v = helics_core_type_tcp()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812628);
  end
  v = vInitialized;
end
