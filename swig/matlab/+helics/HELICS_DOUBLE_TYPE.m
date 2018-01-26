function v = HELICS_DOUBLE_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 34);
  end
  v = vInitialized;
end
