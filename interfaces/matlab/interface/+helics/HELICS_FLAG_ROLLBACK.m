function v = HELICS_FLAG_ROLLBACK()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 36);
  end
  v = vInitialized;
end
