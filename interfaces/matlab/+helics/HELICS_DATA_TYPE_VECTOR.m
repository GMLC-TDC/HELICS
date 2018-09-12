function v = HELICS_DATA_TYPE_VECTOR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230899);
  end
  v = vInitialized;
end
