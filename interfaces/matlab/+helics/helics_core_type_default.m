function v = helics_core_type_default()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812622);
  end
  v = vInitialized;
end
